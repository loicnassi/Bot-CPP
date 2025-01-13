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
    contract.symbol = "NVDL";
    contract.currency = "USD";
    contract.secType = "STK";
    contract.exchange = "ARCA";
//    contract.primaryExchange = "SBF";
    
    Contract contract2 = Contract();
    contract2.symbol = "NVDX";
    contract2.currency = "USD";
    contract2.secType = "STK";
    contract2.exchange = "ARCA";
//    contract2.primaryExchange = "SBF";
    
//    Connection to the IBApp
    
    App app("ib-gateway", 4004);
    app.wait("connect");
    
    Portfolio portfolio(&app, 1);
    
    Asset endogene(&app, contract, "MIDPOINT");
    Asset exogene(&app, contract2, "MIDPOINT");
    
    std::vector<Asset*> assets{&endogene, &exogene};
    Basket basket(assets);
    std::vector<Basket*> baskets{&basket};
    
    //    Params strategy
    std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>>>> params;
        
    params[basket.name] = {
        {"HedgeRatio", std::vector<double>{1.0, -1.0}},
        {"Barsize", 5},
        {"Lookback", 10.0},
        {"Threshold", 1.0},
        {"Exit", 1.0},
        {"Slippage", 1.0},
        {"Security", 1.0},
        {"Costs", 0.005},
        {"MeanDrift", 0.00055},
        {"VolatilityDrift", 0.00015}
    };
    
    BasketTradingBot basket_bot = BasketTradingBot(baskets, &app);
    basket_bot.launch(params);
    
    app.wait();
    app.eDisconnect();

    //    For now error 509 occures in disconnecting process. A reason could be the multiple requests for closing socket. It doesn't interrupt the programm in other functions.
    return 0;
}
