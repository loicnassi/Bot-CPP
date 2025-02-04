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
    Portfolio(App &app, const double leverage, const std::string accounts = "All");
    ~Portfolio();
    
    struct Trade {
        std::string symbol;
        std::string side;
        double quantity;
        double avgPrice;
        double currentPrice;
        double currentPnl;
        std::optional<Contract> contract;
        std::optional<Execution> execution;
        std::optional<CommissionReport> commissionReport;
    };
//  Major features
    App &app;
    
    const std::string accounts;
    const double leverage;
    std::unordered_map<std::string, Trade> portfolioPositions;
    std::unordered_map<std::string, std::string> summary; // Summary data arrive as string function std::stod() when needed to transform in double
    
    void getAccountSummary(std::string tags);
    void getPositions();
    void computePnl(Trade &trade);
    void fit(Asset &asset);
    
};

#endif /* Portfolio_hpp */
