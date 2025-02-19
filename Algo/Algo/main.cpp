//
//  main.cpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#include "App.hpp"
#include "Asset.hpp"
#include "Portfolio.hpp"
#include "Strategies/BasketTrading/BasketTradingBot.hpp"
#include "Strategies/BasketTrading/Basket.hpp"


int main()
{
    
    // Log the start of initialization
    Logger& logger = Logger::getInstance();
    logger.setLogFile("log");
    logger.enableConsoleLogging(true);
    logger.setLogLevel(Logger::Level::DETAIL);
    
//    Contracts definition
    Contract contract = Contract();
    contract.symbol = "CSX";
    contract.currency = "USD";
    contract.secType = "STK";
    contract.exchange = "SMART";
//    contract.primaryExchange = "NASDAQ";
    
    Contract contract2 = Contract();
    contract2.symbol = "TWM";
    contract2.currency = "USD";
    contract2.secType = "STK";
    contract2.exchange = "SMART";
//    contract2.primaryExchange = "NASDAQ";
    
//    Connection to the IBApp
    App app("localhost", 7497);
    app.wait("connect");
    
    Portfolio portfolio(app, 1);
    
    Asset endogene(app, contract, "MIDPOINT");
    Asset exogene(app, contract2, "MIDPOINT");
    
    std::vector<Asset*> assets{&endogene, &exogene};
    Basket basket(assets);
    std::vector<Basket*> baskets{&basket};
    
    //    Params strategy
    std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params;
        
    params[basket.name] = {
        {"HedgeRatio", std::vector<double>{1.0, 0.35}},
        {"BarSizeConsolidation", 5},
        {"BarSize", "5 secs"},
        {"DurationStr", "8280 S"},
        {"Lookback", 1655.0}, // This variable has to be set as a double ".0", if not --> error
        {"Threshold", 1.0},
        {"Exit", 1.0},
        {"Slippage", 1.0},
        {"Security", 2.0},
//        {"Stop Loss", 0.0},
        {"Costs", 0.005},
        {"MeanDrift", 0.0},
        {"VolatilityDrift", 0.0}
    };
    
    BasketTradingBot basket_bot = BasketTradingBot(baskets, &app);
    basket_bot.launch(params, true);
//    
    app.wait();
    app.eDisconnect();

    //    For now error 509 occures in disconnecting process. A reason could be the multiple requests for closing socket. It doesn't interrupt the programm in other functions.
    return 0;
}
