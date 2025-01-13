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
void Asset::getRealTime(int barSize, bool userRTH) {
    
    this->barSize = barSize;
    
    app->reqRealTimeBars(app->requestId, contract, barSize, whatToShow, userRTH, TagValueListSPtr());
    app->requests[app->requestId] = this;
    app->requestId ++;
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
        order.orderId = app->orderId++;
        position += orderQuantity;
    }
    app->placeOrder(order.orderId, contract, order);
}
    
void Asset::orderMarketSell(double orderQuantity, bool simulated) {
    
    Order order;
    order.action = "SELL";
    order.orderType = "MKT";
    order.totalQuantity = DecimalFunctions::doubleToDecimal(orderQuantity);
    order.outsideRth = true;
    order.whatIf = simulated;
    
    if (not simulated) {
        order.orderId = app->orderId++;
        position -= orderQuantity;
    }
    app->placeOrder(order.orderId, contract, order);
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
        order.orderId = app->orderId++;
        position += orderQuantity;
    }
    app->placeOrder(order.orderId, contract, order);
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
        order.orderId = app->orderId++;
        position -= orderQuantity;
    }
    app->placeOrder(order.orderId, contract, order);
    
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
}

void Asset::fit() {
    
//function to retrieve historical data and prepare Asset

}

void Asset::computeReturns() {
    
    unsigned long pricesSize = prices.size();
    
    if (pricesSize > 1) {
        double priceLast = lastPrice.close;
        double pricePrevious = prices[pricesSize - 2].close;
        returns.push_back(log(priceLast/pricePrevious));
        
        if (returns.size() > lookback) {
            returns.erase(returns.begin());
        }
    }
}

void Asset::computeReturnsMean() {
    
    unsigned long returnsSize = returns.size();
    returnsMean = std::reduce(returns.begin(), returns.end()) / returnsSize;
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Returns Mean = " << returnsMean;
    app->logs.printLog(logStream.str());
}

void Asset::computeReturnsStd() {
    
    double accum = 0.0;
    
    std::for_each (std::begin(returns), std::end(returns), [&](const double value) {
        accum += (value - returnsMean) * (value - returnsMean);
    });
    
    returnsStd = sqrt(accum / (returns.size()-1));
    
    std::ostringstream logStream;
    logStream << contract.symbol <<  " | Returns Volatility = " << returnsStd;
    app->logs.printLog(logStream.str());
}

double Asset::computeSlippage() {
    
    double slippage = exp(returnsStd) - 1;
    
    return slippage;
}

double Asset::estimatedCosts(double positionQuantity, double transactionCosts) {
    
    double positionAmount = positionQuantity * lastPrice.close;
    double costs = fmin(fmax(positionQuantity * transactionCosts, 1.0)/positionAmount, 0.01) * (positionAmount);
    
    return costs;
}
