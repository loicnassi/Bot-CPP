//
//  BasketTradingBot.cpp
//  Algo
//
//  Created by Loïc Nassi on 18/12/2024.
//

#include "BasketTradingBot.hpp"

BasketTradingBot::BasketTradingBot(std::vector<Basket*> baskets, App *app):

    app(app),
    baskets(baskets),
    params()
    {
        app->storedBasketStrategy = this;
        portfolio = app->storedPortfolio;
        
        for(std::size_t i = 0; i < baskets.size(); ++i) {baskets[i]->storedStrategy = this;
        };
    }

BasketTradingBot::~BasketTradingBot() {
}


//Pair Trading Bot function. Forward declaration in the App.hpp file
void Basket::tradingStrategy(Basket *basket) {

    const std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>> &ref(storedStrategy->params[basket->name]);
    const double &exit = std::get<double>(ref.at("Exit"));
    const double &threshold = std::get<double>(ref.at("Threshold"));
    const double &security = std::get<double>(ref.at("Security"));
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Current Position: {}", name, position));
    
    if ((position == 1 && spreadsZScore > (exit * threshold)) || (position == -1 && spreadsZScore < (-exit * threshold))) {
            closePositions();
        }
    
    if (position == 0 && storedStrategy->estimatedPnl(basket) >= security) {
            if (spreadsZScore > threshold) {
                orderMarketShort();
            }
            else if (spreadsZScore < -threshold) {
                orderMarketLong();
            }
        }
    }

void BasketTradingBot::fit(std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params) {
    
    this->params = params;
    computeCapitalAllocation();
    
    std::vector<std::future<void>> basket_futures;
    
    for(std::size_t i = 0; i < baskets.size(); ++i) {
        basket_futures.push_back(std::async(std::launch::async, [&, i] {
            const auto &ref(params[baskets[i]->name]);
            baskets[i]->basketLookBack = std::get<double>(ref.at("Lookback"));
            baskets[i]->hedgeRatio = std::get<std::vector<double>>(ref.at("HedgeRatio"));
            baskets[i]->spreadMeanDrift = std::get<double>(ref.at("MeanDrift"));
            baskets[i]->spreadStdDrift = std::get<double>(ref.at("VolatilityDrift"));
        
            std::vector<std::future<void>> asset_futures;
            
            for (std::size_t j = 0; j < baskets[i]->assets.size(); ++j) {
                asset_futures.push_back(std::async(std::launch::async, [&, i, j] {

                    baskets[i]->assets[j]->lookback = std::get<double>(ref.at("Lookback"));
                    baskets[i]->assets[j]->getHistorical(std::get<std::string>(ref.at("BarSize")), std::get<std::string>(ref.at("DurationStr")), "", true, 0);
                }));
            }
            for (auto &future : asset_futures) { future.get();}
        }));
    }
    
    for (auto &future : basket_futures) { future.get(); }
    
    for (std::size_t i = 0; i < baskets.size(); ++i) {
        for (std::size_t k = 0; k < baskets[i]->assets[0]->prices.size(); ++k) {
            for (std::size_t j = 0; j < baskets[i]->assets.size(); ++j) {
                baskets[i]->assets[j]->lastPrice = baskets[i]->assets[j]->prices[k];
            }
            baskets[i]->computeSpread();
        }
        baskets[i]->computeSpreadZScore();
        baskets[i]->computeSpreadDrift();
    }
}

void BasketTradingBot::launch() {
    
    for(std::size_t i = 0; i < baskets.size(); ++i) {
        const auto &ref(params[baskets[i]->name]);
        
        for (std::size_t j = 0; j < baskets[i]->assets.size(); ++j) {
            baskets[i]->assets[j]->getRealTime(std::get<int>(ref.at("BarSizeConsolidation")));
        }
        
        Logger::getInstance().log(Logger::Level::INFO,
            std::format(
                "=============================\n"
                "Launch Basket Trading Strategy\n"
                "=============================\n"
                "Barsize : {}\n"
                "Lookback Period : {}\n"
                "Threshold : {}\n"
                "Exit : {}\n"
                "Slippage : {}\n"
                "Security : {}\n"
                "Transaction Costs : {}\n"
                "=============================",
                std::get<std::string>(ref.at("BarSize")),
                std::get<double>(ref.at("Lookback")),
                std::get<double>(ref.at("Threshold")),
                std::get<double>(ref.at("Exit")),
                std::get<double>(ref.at("Slippage")),
                std::get<double>(ref.at("Security")),
                std::get<double>(ref.at("Costs"))));
    }
}

void BasketTradingBot::computeCapitalAllocation() {
    
    portfolio->getAccountSummary("TotalCashValue");
    double buyingPower = std::stod(portfolio->summary["TotalCashValue"]) * portfolio->leverage;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("Buying Power : {}", buyingPower));
                              
    for(std::size_t i = 0; i < baskets.size(); ++i) {
        baskets[i]->capitalAllocation = std::floor(buyingPower / baskets.size());
        
        Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Capital Allocation: {}", baskets[i]->name, baskets[i]->capitalAllocation));
    };
}

void Basket::computeSpreadDrift() {
    
    spreadDrift = exp(spreadMeanDrift + std::get<double>(storedStrategy->params[name]["Threshold"]) * spreadStdDrift) - 1;

    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Spread Drift: {}", name, spreadDrift));
}

double BasketTradingBot::computeCosts(Basket *basket) {
    
    const double &slippageRate = std::get<double>(params[basket->name]["Slippage"]);
    const double &transactionCostRate = std::get<double>(params[basket->name]["Costs"]);
    double slippage = 0.0, transaction = 0.0;
    
    for (std::size_t i = 0; i < basket->assets.size(); ++i) {
        const auto& asset = basket->assets[i];
        slippage += asset->computeSlippage() * asset->lastPrice.close * abs(basket->quantities[i]) * slippageRate;
        transaction += asset->estimatedCosts(basket->quantities[i], transactionCostRate);
    }

    const double drift = basket->spreadDrift * basket->capitalAllocation * slippageRate;
    const double costs = 2 * (slippage + transaction) + drift;

    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Slippage: {} / Drift: {} / Transaction Costs: {}", basket->name, 2 * slippage, drift, 2 * transaction));
    
    return costs;
}

double BasketTradingBot::estimatedPnl(Basket *basket) {
    
    basket->computeQuantites();
    double investment = 0.0;
    
    for(std::size_t i = 0; i < basket->assets.size(); ++i) {
        investment += basket->assets[i]->lastPrice.close * abs(basket->quantities[i]);
    }
    
    const int spreadsZScoreSign = (basket->spreadsZScore > 0) - (basket->spreadsZScore < 0);
    double profit = (exp(abs(basket->spreadStd * (basket->spreadsZScore - spreadsZScoreSign * (std::get<double>(params[basket->name]["Threshold"]) * -std::get<double>(params[basket->name]["Exit"]))))) - 1) * investment;
    double loss = computeCosts(basket);
    double pnl = profit / loss;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | PNL: {} - {} = {} | PNL Ratio: {}", basket->name, profit, loss, profit - loss, pnl));
    
    return pnl;
}
