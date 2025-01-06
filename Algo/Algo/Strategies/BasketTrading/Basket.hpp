//
//  Basket.hpp
//  Algo
//
//  Created by Loïc Nassi on 18/12/2024.
//

#ifndef Basket_hpp
#define Basket_hpp

#include <stdio.h>
#include <iostream>

#include <vector>
#include <numeric>
#include <algorithm>

#include "../../Asset.hpp"


//  Forward declaration
class BasketTradingBot;

class Basket {
    
public:
    Basket(std::vector<Asset*> assets);
    ~Basket();
    
    //  Major features
    std::vector<Asset*> assets;
    std::string name;
    BasketTradingBot *storedStrategy;
    
    std::vector<double> spreads;
    double spreadsZScore;
    double basketLookBack;
    std::vector<double> hedgeRatio;
    
    double spreadMean;
    double spreadStd;
    double spreadMeanDrift;
    double spreadStdDrift;
    double spreadDrift;
    
    std::vector<int> quantities;
    
    double capitalAllocation;
    int position;
    
    //  Strategy link functions
    void computeSpread();
    void computeSpreadMean();
    void computeSpreadStd();
    void computeSpreadZScore();
    void computeSpreadDrift();
    void computeQuantites();
    
    void basketPipeline();
    void tradingStrategy(Basket *basket); // Forward declaration, function is declared in BasketTradingBot.cpp
    
    //  Order Management
    void orderMarketLong();
    void orderMarketShort();
    void orderLimitLong(std::vector<double> orderPrices);
    void orderLimitShort(std::vector<double> orderPrices);
    void closePositions();
};
    
#endif /* Basket_hpp */
