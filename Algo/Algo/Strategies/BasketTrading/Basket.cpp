//
//  Basket.cpp
//  Algo
//
//  Created by Loïc Nassi on 18/12/2024.
//

#include "Basket.hpp"

Basket::Basket(std::vector<Asset*> assets):

//  Major features
    assets(assets),
    storedStrategy(nullptr),

    spreads(),
    spreadsZScore(),
    basketLookBack(),
    hedgeRatio(),

    spreadMean(),
    spreadStd(),
    spreadMeanDrift(),
    spreadStdDrift(),
    spreadDrift(),
    quantities(),
    

    position(0),
    capitalAllocation() {
        for (std::size_t i = 0; i < assets.size(); ++i) {
            name += assets[i]->contract.symbol;
                if (i != assets.size() - 1) {
                    name += "-";
                }
        };
        
        for(std::size_t i = 0; i < assets.size(); ++i) {
            assets[i]->storedBasket = this;
        };
    }

Basket::~Basket() {
}

void App::handleBar(long reqId, Asset *asset, Bar bar) {

    asset->barConsolidation += 5;
    if (asset->barConsolidation == 5) {
        asset->lastPrice = bar;
    }
    else {
        asset->lastPrice.time = bar.time;
        asset->lastPrice.high = fmax(bar.high, asset->lastPrice.high);
        asset->lastPrice.low = fmin(bar.low, asset->lastPrice.low);
        asset->lastPrice.close = bar.close;
        asset->lastPrice.volume += bar.volume;
        asset->lastPrice.wap = ((asset->lastPrice.wap * (asset->lastPrice.volume - bar.volume)) + (bar.wap * bar.volume)) / asset->lastPrice.volume;
        }
    
    int barTime = std::stoi(bar.time);
    int barStartTime = (barTime / asset->barSize) * asset->barSize;

    if (barTime == (barStartTime+asset->barSize-5)) {
        if (asset->barConsolidation == asset->barSize) {
            asset->prices.push_back(asset->lastPrice);
            
            if (asset->prices.size() > asset->storedBasket->basketLookBack) {
                asset->prices.erase(asset->prices.begin());
            }
            
            Logger::getInstance().log(Logger::Level::INFO, std::format("{}  | Price Last | Time: {} | Open: {} | Close: {}", asset->contract.symbol, bar.time, bar.open, bar.close));
            
            if (asset->storedBasket) {
                asset->storedBasket->basketPipeline(false);
            }
        }
        asset->barConsolidation = 0;
    }
}

void App::fitBar(long reqId, Asset *asset, Bar bar, bool update) {
    
    if (update==true) {
        
        if (bar.time == asset->lastPrice.time) {
            asset->prices.erase(--asset->prices.end());
            asset->prices.push_back(bar);
        }
        else {
            cancelHistoricalData(reqId);
            retrieved[reqId]=true;
        }
    }
    else {
        asset->prices.push_back(bar);
        
        if (asset->prices.size() > asset->lookback) {
            asset->prices.erase(asset->prices.begin());
        }
    }
    asset->lastPrice = bar;
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Price Last Fit | Time: {} | Open: {} | Close: {}", asset->contract.symbol, bar.time, bar.open, bar.close));
}


// Strategy link functions
void Basket::computeSpread() {
    
    double spread=0.0;
    
    for(std::size_t i = 0; i < assets.size(); ++i) {
        const Bar &assetPricesLast = assets[i]->lastPrice;
        spread += log(assetPricesLast.close) * hedgeRatio[i];
    };
    
    spreads.push_back(spread);
    
    if (spreads.size() > basketLookBack) {
        spreads.erase(spreads.begin());
    }

    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Spread: {}", name, spread));
}

void Basket::computeSpreadMean() {
    
    spreadMean = std::reduce(spreads.begin(), spreads.end()) / spreads.size() ;
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Spread Mean: {}", name, spreadMean));
}

void Basket::computeSpreadStd() {
    
    double accum = 0.0;
    
    std::for_each(std::begin(spreads), std::end(spreads), [&](const double value) {
        accum += (value - spreadMean) * (value - spreadMean);
    });
    
    spreadStd = sqrt(accum / (spreads.size()-1));

    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Spread Volatility: {}", name, spreadStd));
}

void Basket::computeSpreadZScore() {
    
    double spread = spreads[spreads.size() - 1];
    computeSpreadMean();
    computeSpreadStd();
    spreadsZScore = (spread - spreadMean)/spreadStd;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Spread Zscore: {}", name, spreadsZScore));
}

void Basket::computeQuantites() {
    
    double quantity = capitalAllocation / std::inner_product(
         hedgeRatio.begin(), hedgeRatio.end(), assets.begin(), 0.0, std::plus<>(), [](double ratio, const auto& asset)
                 { return std::abs(ratio) * asset->lastPrice.close; }
                                                             );
    quantities.resize(hedgeRatio.size());
    
    std::transform(hedgeRatio.begin(), hedgeRatio.end(), assets.begin(), quantities.begin(), [&](double ratio, const auto& asset) {
        double qty = std::trunc(ratio * quantity);

        Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Quantity: {}", name, qty));
        
        return qty;
    }
                   );
}

void Basket::basketPipeline(bool fit) {
    
    std::vector<std::string> pricesTimes;
    std::vector<unsigned long> pricesSizes;
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        pricesTimes.push_back(assets[i]->lastPrice.time);
        pricesSizes.push_back(assets[i]->prices.size());
    }
    
    if (std::equal(pricesTimes.begin() + 1, pricesTimes.end(), pricesTimes.begin()) && std::equal(pricesSizes.begin(), pricesSizes.end(), std::vector<int>(pricesSizes.size(), basketLookBack).begin())) {
        
        for (std::size_t i = 0; i < assets.size(); ++i) {
            assets[i]->computeReturns();
            assets[i]->computeReturnsMean();
            assets[i]->computeReturnsStd();
        }
        
        computeSpread();
        computeSpreadZScore();
        computeSpreadDrift();
        if (not fit) {
            tradingStrategy(this);
        }
    }
}

//Order Management
void Basket::orderMarketLong() {
    
    computeQuantites();
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        if (quantities[i] > 0){
            assets[i]->orderMarketBuy(quantities[i]);
        }
        else if (quantities[i] < 0)
            assets[i]->orderMarketSell(-quantities[i]);
    }
    position = 1;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Basket Market Long Order", name));
}

void Basket::orderMarketShort() {
    
    computeQuantites();
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        if (-quantities[i] > 0){
            assets[i]->orderMarketBuy(quantities[i]);
        }
        else if (-quantities[i] < 0)
            assets[i]->orderMarketSell(quantities[i]);
    }
    
    position = -1;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Basket Market Short Order", name));
}

void Basket::orderLimitLong(std::vector<double> orderPrices) {
    
    computeQuantites();
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        if (quantities[i] > 0){
            assets[i]->orderLimitBuy(orderPrices[i], quantities[i]);
        }
        else if (quantities[i] < 0)
            assets[i]->orderMarketSell(-quantities[i]);
    }
    
    position = 1;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Basket Limit Long Order", name));
}

void Basket::orderLimitShort(std::vector<double> orderPrices) {
    
    computeQuantites();
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        if (-quantities[i] > 0){
            assets[i]->orderLimitBuy(orderPrices[i], quantities[i]);
        }
        else if (-quantities[i] < 0)
            assets[i]->orderMarketSell(quantities[i]);
    }
    
    position = -1;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Basket Limit Short Order", name));
}

void Basket::closePositions() {
    
    computeQuantites();
    
    for (std::size_t i = 0; i < assets.size(); ++i) {
        assets[i]->closePositions();
    }
    
    position = 0;
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Basket Limit Close Order", name));
}
