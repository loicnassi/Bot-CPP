//
//  Asset.cpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#include "Asset.hpp"


Asset::Asset(App &app, Contract &contract, std::string whatToShow):

//  Major features
    app(app),
    contract(contract),
    storedBasket(nullptr),
    
    whatToShow(whatToShow),
    barSize(),
    barConsolidation(),
    barStartTime(),
    lookback(),

    lastPrice(),
    prices(),
    returns(),
    returnsMean(),
    returnsStd(),
    position(0) {
    }

Asset::~Asset() {
}

//Data handling
void Asset::getRealTime(int barSize, bool useRTH) {
    
    double id = app.incrementRequestId();
    app.requests[id] = this;
    this->barSize = barSize;
    app.reqRealTimeBars(id, contract, barSize, whatToShow, useRTH, TagValueListSPtr());
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Retrieve Real Time Bar | Bar Size : {}", contract.symbol, barSize));
}

void Asset::getHistorical(std::string barSize, std::string durationStr, std::string endDateTime, bool keepUpdate, int useRTH) {
    
    double id = app.incrementRequestId();
    app.requests[id] = this;
    app.reqHistoricalData(id, contract, endDateTime, durationStr, barSize, whatToShow, useRTH, 2, keepUpdate, TagValueListSPtr());
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("{} | Retrieve Historical Data | Bar Size : {} | Duration : {}", contract.symbol, barSize, durationStr));
    app.wait("retrieve", id);
}

//Order Management
void Asset::orderMarketBuy(double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "BUY";
    order.orderType = "MKT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.outsideRth = true;
    order.whatIf = simulated;
    
    if (not simulated) {
        double id = app.incrementOrderId();
        order.orderId = id;
        position += orderQuantity;
    }
    else {
        order.orderId = app.orderId;
    }
    app.placeOrder(order.orderId, contract, order);
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Market Order Buy | Quantity : {}", contract.symbol, orderQuantity));
}
    
void Asset::orderMarketSell(double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "SELL";
    order.orderType = "MKT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.outsideRth = true;
    order.whatIf = simulated;
    
    if (not simulated) {
        double id = app.incrementOrderId();
        order.orderId = id;
        position -= orderQuantity;
    }
    else {
        order.orderId = app.orderId;
    }
    app.placeOrder(order.orderId, contract, order);
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Market Order Sell | Quantity : {}", contract.symbol, orderQuantity));
}

void Asset::orderLimitBuy(double orderPrice, double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "BUY";
    order.orderType = "LMT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.lmtPrice = orderPrice;
    order.outsideRth = true;
    order.whatIf = simulated;
    
    
    if (not simulated) {
        double id = app.incrementOrderId();
        order.orderId = id;
        position += orderQuantity;
    }
    else {
        order.orderId = app.orderId;
    }
    app.placeOrder(order.orderId, contract, order);
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Limit Order Buy | Price : {} | Quantity : {}", contract.symbol, orderPrice, orderQuantity));
}
    
void Asset::orderLimitSell(double orderPrice, double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "SELL";
    order.orderType = "LMT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.lmtPrice = orderPrice;
    order.outsideRth = true;
    order.whatIf = simulated;
    
    if (not simulated) {
        double id = app.incrementOrderId();
        order.orderId = id;
        position -= orderQuantity;
    }
    else {
        order.orderId = app.orderId;
    }
    app.placeOrder(order.orderId, contract, order);
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Limit Order Sell | Price : {} | Quantity : {}", contract.symbol, orderPrice, orderQuantity));
}

void Asset::closePositions(double orderQuantity, bool simulated) {
        
    if (orderQuantity == 0) {
        orderQuantity = position;
    }
    
    if (position > 0) {
        orderMarketSell(orderQuantity, simulated);
    }
    else if (position < 0) {
        orderMarketBuy(-orderQuantity, simulated);
    }
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | [Close Positions]", contract.symbol));
}

void Asset::computeReturns() {
    
    unsigned long pricesSize = prices.size();
    
    if (pricesSize > 1) {
        double priceLast = lastPrice.close;
        double pricePrevious = prices[pricesSize - 2].close;
        double priceReturn = log(priceLast/pricePrevious);
        returns.push_back(priceReturn);
        
        if (returns.size() > lookback) {
            returns.erase(returns.begin());
        }
        
        Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Return : {}", contract.symbol, priceReturn));
    }
}

void Asset::computeReturnsMean() {
    
    unsigned long returnsSize = returns.size();
    returnsMean = std::reduce(returns.begin(), returns.end()) / returnsSize;
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Returns Mean : {}", contract.symbol, returnsMean));
}

void Asset::computeReturnsStd() {
    
    double accum = 0.0;
    
    std::for_each (std::begin(returns), std::end(returns), [&](const double value) {
        accum += (value - returnsMean) * (value - returnsMean);
    });
    
    returnsStd = sqrt(accum / (returns.size()-1));
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Returns Volatility : {}", contract.symbol, returnsStd));
}

double Asset::computeSlippage() {
    
    double slippage = exp(returnsStd) - 1;
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Slippage : {}", contract.symbol, slippage));
    
    return slippage;
}

double Asset::estimatedCosts(double positionQuantity, double transactionCosts) {
    
    double positionAmount = positionQuantity * lastPrice.close;
    double costs = fmin(fmax(positionQuantity * transactionCosts, 1.0)/positionAmount, 0.01) * (positionAmount);
    
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Costs : {}", contract.symbol, costs));
    
    return costs;
}
