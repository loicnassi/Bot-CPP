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
        
        for(std::size_t i = 0; i < baskets.size(); ++i) baskets[i]->storedStrategy = this;
    }

BasketTradingBot::~BasketTradingBot() {
}


//Pair Trading Bot function. Forward declaration in the App.hpp file
void App::fetchAsset(const Contract &contract, std::string side, double quantity) {
    
    for (Basket* basket: storedBasketStrategy->baskets) {
        for (Asset* asset: basket->assets) {
            if (asset->contract.symbol == contract.symbol) {
                asset->position = (side == "BOT") ? quantity :-quantity;
            }
        }
        std::size_t invested = 0;
        for (Asset* asset: basket->assets) {
            if (asset->position > 0.0 || asset->position < 0.0) {
                invested +=1;
                Logger::getInstance().log(Logger::Level::DETAIL, std::format("Asset {} | Assets size {} | invested {}", asset->contract.symbol, basket->assets.size(), invested));
            }
        }
            
        if (invested == basket->assets.size()) {
            basket->position = (basket->assets[0]->position > 0) ? 1 : -1;
            Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Fetch Position: {}", basket->name, basket->position));
        }
    }
}

void Basket::tradingStrategy(Basket *basket) {

    const std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>> &ref(storedStrategy->params[basket->name]);
    const double &exit = std::get<double>(ref.at("Exit"));
    const double &threshold = std::get<double>(ref.at("Threshold"));
    const double &security = std::get<double>(ref.at("Security"));
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Current Position: {}", name, position));
    
    const double exitThreshold = exit * threshold;
    if (position == 1 || position == -1) {
        basket->computePnl();
        if ((position == 1 && spreadsZScore > exitThreshold) || (position == -1 && spreadsZScore < -exitThreshold)) {
            closePositions();
            storedStrategy->computeCapitalAllocation();
        }
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
    portfolio->getPositions();
    
    std::vector<std::future<void>> basket_futures;
    
    for(Basket* basket : baskets) {
        basket_futures.push_back(std::async(std::launch::async, [basket, this, &params] {
            const auto &ref(params[basket->name]);
            basket->basketLookBack = std::get<double>(ref.at("Lookback"));
            basket->hedgeRatio = std::get<std::vector<double>>(ref.at("HedgeRatio"));
            basket->spreadMeanDrift = std::get<double>(ref.at("MeanDrift"));
            basket->spreadStdDrift = std::get<double>(ref.at("VolatilityDrift"));
            std::vector<std::future<void>> asset_futures;
            
            for (Asset* asset : basket->assets) {
                asset_futures.push_back(std::async(std::launch::async, [asset, &ref] {
                    asset->lookback = std::get<double>(ref.at("Lookback"));
                    asset->getHistorical(std::get<std::string>(ref.at("BarSize")), std::get<std::string>(ref.at("DurationStr")), "", true, 0);
                }));
            }
            
            for (auto &future : asset_futures) { future.get(); }
        }));
    }
    
    for (auto &future : basket_futures) { future.get(); }
    
    for (Basket* basket : baskets) {
        for (std::size_t k = 0; k < basket->assets[0]->prices.size(); ++k) {
            for (Asset* asset : basket->assets) {
                asset->lastPrice = asset->prices[k];
            }
            basket->computeSpread();
        }
        basket->computeSpreadZScore();
        basket->computeSpreadDrift();
    }
}

void BasketTradingBot::launch(std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params, bool fit) {
    
    if (fit) this->fit(params);
    else {
        this->params = params;
        computeCapitalAllocation();
        portfolio->getPositions();
    }
    
    for(Basket* basket : baskets) {
        const auto &ref(params[basket->name]);
        if (not fit) {
            const auto &ref(params[basket->name]);
            basket->basketLookBack = std::get<double>(ref.at("Lookback"));
            basket->hedgeRatio = std::get<std::vector<double>>(ref.at("HedgeRatio"));
            basket->spreadMeanDrift = std::get<double>(ref.at("MeanDrift"));
            basket->spreadStdDrift = std::get<double>(ref.at("VolatilityDrift"));
        };
        
        for (Asset* asset : basket->assets) {
            if (not fit) {
                asset->lookback = std::get<double>(ref.at("Lookback"));
            };
            asset->getRealTime(std::get<int>(ref.at("BarSizeConsolidation")));
        }
        
        Logger::getInstance().log(Logger::Level::INFO,
            std::format(
                "\n===============================\n"
                "Launch Basket Trading Strategy\n"
                "===============================\n"
                "Barsize: {}\n"
                "Lookback Period: {}\n"
                "Threshold: {}\n"
                "Exit: {}\n"
                "Slippage: {}\n"
                "Security: {}\n"
                "Transaction Costs: {}\n"
                "===============================",
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
    
    portfolio->getAccountSummary("NetLiquidation");
    double buyingPower = std::stod(portfolio->summary["NetLiquidation"]) * portfolio->leverage;
    double allocation = std::floor(buyingPower / baskets.size());
    Logger::getInstance().log(Logger::Level::INFO, std::format("Buying Power: {}", buyingPower));
                              
    for(Basket* basket : baskets) {
        basket->capitalAllocation = allocation;
        Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Capital Allocation: {}", basket->name, allocation));
    };
}

void Basket::computeSpreadDrift() {
    
    spreadDrift = spreadMeanDrift + std::get<double>(storedStrategy->params[name]["Threshold"]) * spreadStdDrift;
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Spread Drift: {}", name, spreadDrift));
}

void Basket::computePnl() {
    
    double pnl = 0.0;
    double initialInvestement = 0.0;
    
    for (Asset *asset : assets) {
        Portfolio::Trade trade(storedStrategy->portfolio->portfolioPositions[asset->contract.symbol]);
        trade.currentPrice = asset->lastPrice.close;
        storedStrategy->portfolio->computePnl(trade);
        
        pnl += trade.currentPnl;
        initialInvestement += (trade.avgPrice * trade.quantity);
    }
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Pnl Position [$]: {} | Pnl Position [%]: {}", name, pnl, pnl/initialInvestement));

    const auto &ref(storedStrategy->params[name]);
    if ( auto checkStopLoss = ref.find("Stop Loss"); checkStopLoss != ref.end()) {
        const double &stopLoss = std::get<double>(ref.at("Stop Loss"));
        if (pnl/initialInvestement < stopLoss) {
            closePositions();
            Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Stop Loss Activated: {}", name, pnl));
        }
    }
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
    const double &normalization = basket->quantities[0];
    
    const auto& ref = params[basket->name];
    const double &threshold = std::get<double>(ref.at("Threshold"));
    const double &exit = std::get<double>(ref.at("Exit"));
    double spreadsZScoreSign = std::copysign(1.0, basket->spreadsZScore);
    
    double profit = abs(basket->spreadStd * (basket->spreadsZScore - spreadsZScoreSign * (threshold * -exit))) * normalization;
    double loss = computeCosts(basket);
    double pnl = profit / loss;

    Logger::getInstance().log(Logger::Level::INFO, format("{} | PNL: {} - {} = {} | PNL Ratio: {}", basket->name, profit, loss, profit - loss, pnl));
    
    return pnl;
}
