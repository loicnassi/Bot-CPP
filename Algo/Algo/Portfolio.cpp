//
//  Portfolio.cpp
//  Algo
//
//  Created by Loïc Nassi on 13/10/2023.
//

#include "Portfolio.hpp"


Portfolio::Portfolio(App &app, const double leverage, const std::string accounts):

//  Major features
    app(app),
    accounts(accounts),
    leverage(leverage),
    summary(),
    portfolioPositions{} {
        
        app.storedPortfolio = this;
        getAccountSummary("NetLiquidation");
}

Portfolio::~Portfolio() {
}


//  App function. Forward declaration in the App.hpp file
void App::handleSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
    
    storedPortfolio->summary[tag] = value;
    Logger::getInstance().log(Logger::Level::INFO, std::format("Acct Summary | ReqId: {}, Account : {}, {}: {} {}", reqId, account, tag, value, currency));
}

void App::handleOrders(int reqId, const Contract &contract, const Execution &execution) {
    
    if (DecimalFunctions::decimalToDouble(execution.cumQty) == 0) {
        storedPortfolio->portfolioPositions.erase(contract.symbol);
    }
    else {
        Portfolio::Trade trade{contract.symbol, execution.side, DecimalFunctions::decimalToDouble(execution.cumQty), execution.avgPrice, 0, 0, contract, execution};
        storedPortfolio->portfolioPositions[contract.symbol] = trade;
        }
}

void App::handlePositions(const Contract &contract, Decimal position, double avgCost) {
    
    std::string side = (DecimalFunctions::decimalToDouble(position) > 0) ? "BOT" : "SLD";
    Portfolio::Trade trade{contract.symbol, side, DecimalFunctions::decimalToDouble(position), avgCost, 0, 0, contract};
    storedPortfolio->portfolioPositions[contract.symbol] = trade;
    if (storedBasketStrategy) {
        fetchAsset(contract, side, trade.quantity);
    }
}

// Acounts function
void Portfolio::getAccountSummary(std::string tags) {
    
    double id = app.incrementRequestId();
    app.reqAccountSummary(id, accounts, tags);
    app.wait("retrieve", id);
}

void Portfolio::computePnl(Trade &trade) {
    
    int multi = (trade.side == "SLD") ? -1 : 1;
    trade.currentPnl = (trade.currentPrice - trade.avgPrice) * trade.quantity * multi;
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("{} | Quantity: {} | Pnl: {}", trade.symbol, trade.quantity, trade.currentPnl));
}

void Portfolio::getPositions() {
    
    app.reqPositions();
}

