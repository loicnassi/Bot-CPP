//
//  AlgoLogs.hpp
//  Algo
//
//  Created by Loïc Nassi on 29/09/2024.
//

#ifndef AlgoLogs_hpp
#define AlgoLogs_hpp

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>

class AlgoLogs {
    
public:
    
    AlgoLogs();
    ~AlgoLogs();
    
//  Annexe functions
    void saveLog(std::string log);
    void printLog(std::string log);
    std::string getDateTime();
};

#endif /* AlgoLogs_hpp */
