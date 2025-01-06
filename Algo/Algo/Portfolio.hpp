//
//  Portfolio.hpp
//  Algo
//
//  Created by Loïc Nassi on 13/10/2023.
//

#ifndef Portfolio_hpp
#define Portfolio_hpp

#include <stdio.h>

#include "App.hpp"

class Portfolio {
    
public:
    Portfolio(App *app, const double leverage, const std::string accounts = "All");
    ~Portfolio();
    
    struct trade  {
        std::string symbol;
        std::string entryTime;
        double entryPrice;
        int quantity;
        double currentPrice;
        double curentPnL;
        double closeTime;
        double closePrice;
        bool closed;
    };
//  Major features
    App *app;
    
    const std::string accounts;
    const double leverage;
    std::vector<trade> portfolioPositions;
    std::map<std::string, std::string> summary; // Summary data arrive as string function std::stod() when needed to transform in double
    
    void getAccountSummary(std::string tags);
    
};

#endif /* Portfolio_hpp */
