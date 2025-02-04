//
//  BasketTradingBot.hpp
//  Algo
//
//  Created by Loïc Nassi on 18/12/2024.
//

#ifndef BasketTradingBot_hpp
#define BasketTradingBot_hpp

#include "Basket.hpp"
#include "../../Portfolio.hpp"

class BasketTradingBot {
    
public:
    BasketTradingBot(std::vector<Basket*> baskets, App *app);
    ~BasketTradingBot();
  
    App *app;
    Portfolio *portfolio;
    std::vector<Basket*> baskets;
    
    std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params;
    
    void launch(std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params, bool fit = true);
    void fit(std::unordered_map<std::string, std::unordered_map<std::string, std::variant<int, double, std::vector<double>, std::string>>> params);
    
    void computeCapitalAllocation();
    
    double computeCosts(Basket *basket);
    double estimatedPnl(Basket *basket);
    
};

#endif /* BasketTradingBot_hpp */
