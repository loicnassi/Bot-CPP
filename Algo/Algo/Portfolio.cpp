//
//  Portfolio.cpp
//  Algo
//
//  Created by Loïc Nassi on 13/10/2023.
//

#include "Portfolio.hpp"


Portfolio::Portfolio(App *app, const double leverage, const std::string accounts):

//  Major features
    app(app),
    accounts(accounts),
    leverage(leverage),
    summary() {
        
        app->storedPortfolio = this;
        getAccountSummary("TotalCashValue");
}

Portfolio::~Portfolio() {
}


//  App function. Forward declaration in the App.hpp file
void App::handleSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
    
    storedPortfolio->summary[tag] = value;
    
    std::ostringstream logStream;
    logStream << "Acct Summary | ReqId: " << reqId << ", Account : " << account << ", " << tag << " : " << value << " " << currency;
    logs.printLog(logStream.str());
}

void App::handleOrders(int reqId, const Contract &contract, const Execution &execution) {
    
}


// Acounts function
void Portfolio::getAccountSummary(std::string tags) {
    app->incrementRequestId();
    app->reqAccountSummary(app->requestId, accounts, tags);
    app->wait("retrieve", app->requestId);
}
