//
//  Asset.cpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#include "Asset.hpp"


Asset::Asset(App *app, Contract contract, std::string whatToShow):

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
    
    double id = app->incrementRequestId();
    app->requests[id] = this;
    
    this->barSize = barSize;
    app->reqRealTimeBars(id, contract, barSize, whatToShow, useRTH, TagValueListSPtr());
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Retrieve Real Time Bar | Bar Size : " << barSize;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
}

void Asset::getHistorical(std::string barSize, std::string durationStr, std::string endDateTime, bool keepUpdate, int useRTH) {
    
    double id = app->incrementRequestId();
    app->requests[id] = this;
    
    app->reqHistoricalData(id, contract, endDateTime, durationStr, barSize, whatToShow, useRTH, 2, keepUpdate, TagValueListSPtr());
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Retrieve Historical Data | Bar Size : " << barSize << " | Duration : " <<durationStr;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
    
    app->wait("retrieve", id);
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
        double id = app->incrementOrderId();
        order.orderId = id;
        position += orderQuantity;
    }
    else {
        order.orderId = app->orderId;
    }
    app->placeOrder(order.orderId, contract, order);
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Market Order Buy | Quantity : " << orderQuantity;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
}
    
void Asset::orderMarketSell(double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "SELL";
    order.orderType = "MKT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.outsideRth = true;
    order.whatIf = simulated;
    
    if (not simulated) {
        double id = app->incrementOrderId();
        order.orderId = id;
        position -= orderQuantity;
    }
    else {
        order.orderId = app->orderId;
    }
    app->placeOrder(order.orderId, contract, order);
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Market Order Sell | Quantity : " << orderQuantity;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
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
        double id = app->incrementOrderId();
        order.orderId = id;
        position += orderQuantity;
    }
    else {
        order.orderId = app->orderId;
    }
    app->placeOrder(order.orderId, contract, order);
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Limit Order Buy | Price : " << orderPrice << " | Quantity : " << orderQuantity;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
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
        double id = app->incrementOrderId();
        order.orderId = id;
        position -= orderQuantity;
    }
    else {
        order.orderId = app->orderId;
    }
    app->placeOrder(order.orderId, contract, order);
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Limit Order Sell | Price : " << orderPrice << " | Quantity : " << orderQuantity;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
    
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
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | [Close Positions]";
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
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
        std::ostringstream logStream;
        logStream << contract.symbol <<  " | Return : " << priceReturn;
        Logger::getInstance().log(Logger::Level::DETAIL, logStream.str());
    }
}

void Asset::computeReturnsMean() {
    
    unsigned long returnsSize = returns.size();
    returnsMean = std::reduce(returns.begin(), returns.end()) / returnsSize;
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Returns Mean : " << returnsMean;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
}

void Asset::computeReturnsStd() {
    
    double accum = 0.0;
    
    std::for_each (std::begin(returns), std::end(returns), [&](const double value) {
        accum += (value - returnsMean) * (value - returnsMean);
    });
    
    returnsStd = sqrt(accum / (returns.size()-1));
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Returns Volatility : " << returnsStd;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
}

double Asset::computeSlippage() {
    
    double slippage = exp(returnsStd) - 1;
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Slippage : " << returnsStd;
    Logger::getInstance().log(Logger::Level::INFO, logStream.str());
    
    return slippage;
}

double Asset::estimatedCosts(double positionQuantity, double transactionCosts) {
    
    double positionAmount = positionQuantity * lastPrice.close;
    double costs = fmin(fmax(positionQuantity * transactionCosts, 1.0)/positionAmount, 0.01) * (positionAmount);
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Costs : " << costs;
    Logger::getInstance().log(Logger::Level::DETAIL, logStream.str());
    
    return costs;
}
