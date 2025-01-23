//
//  Asset.hpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#ifndef Asset_hpp
#define Asset_hpp

#include <stdio.h>
#include <algorithm>
#include <execution>

#include "App.hpp"


class Asset {
    
public:
    
    Asset(App *app, Contract contract, std::string whatToShow);
    ~Asset();
    
//  Major features
    App *app;
    Contract contract;
    Basket *storedBasket;

    std::string whatToShow;
    int barSize;
    int barConsolidation;
    int barStartTime;
    double lookback;
    
    Bar lastPrice;
    std::vector<Bar> prices;
    std::vector<double> returns;
    double returnsMean;
    double returnsStd;
    double position;
  
//  Data handling
    void getRealTime(int barSize=5, bool useRTH=false);
    void getHistorical(std::string barSize, std::string durationStr, std::string endDateTime, bool keepUpdate, int useRTH);
    
//  Order Management
    void orderMarketBuy(double orderQuantity, bool simulated=false);
    void orderMarketSell(double orderQuantity, bool simulated=false);
    void orderLimitBuy(double orderPrice, double orderQuantity, bool simulated=false);
    void orderLimitSell(double orderPrice, double orderQuantity, bool simulated=false);
    void closePositions(double orderQuantity=0, bool simulated=false);
    
//  Annexe functions
    void computeReturnsMean();
    void computeReturnsStd();
    void computeReturns(); // Forward declaration, function has to be declared in Pairs
    double computeSlippage();
    double estimatedCosts(double positionQuantity, double transactionCosts);
    double amountToQuantity(double orderAmount);
};

#endif /* Asset_hpp */
