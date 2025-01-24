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
    
//    Contracts definition
    Contract contract = Contract();
    contract.symbol = "AGG";
    contract.currency = "USD";
    contract.secType = "STK";
    contract.exchange = "ARCA";
    
    Contract contract2 = Contract();
    contract2.symbol = "BND";
    contract2.currency = "USD";
    contract2.secType = "STK";
    contract2.exchange = "ARCA";
    
//    Connection to the IBApp
    App app("localhost", 7497);
    app.wait("connect");
    
    Portfolio portfolio(&app, 1);
    
    Asset endogene(&app, contract, "MIDPOINT");
    Asset exogene(&app, contract2, "MIDPOINT");
    
    std::vector<Asset*> assets{&endogene, &exogene};
    Basket basket(assets);
    std::vector<Basket*> baskets{&basket};
    
    //    Params strategy
    std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params;
        
    params[basket.name] = {
        {"HedgeRatio", std::vector<double>{1.0, -0.8}},
        {"BarSizeConsolidation", 5},
        {"BarSize", "5 secs"},
        {"DurationStr", "250 S"},
        {"Lookback", 48.0},
        {"Threshold", 1.0},
        {"Exit", 1.0},
        {"Slippage", 1.0},
        {"Security", 1.0},
        {"Costs", 0.005},
        {"MeanDrift", 0.001125},
        {"VolatilityDrift", 0.002915}
    };
    
    BasketTradingBot basket_bot = BasketTradingBot(baskets, &app);
    basket_bot.fit(params);
    basket_bot.launch();
    
    app.wait();
    app.eDisconnect();

    //    For now error 509 occures in disconnecting process. A reason could be the multiple requests for closing socket. It doesn't interrupt the programm in other functions.
    return 0;
}
