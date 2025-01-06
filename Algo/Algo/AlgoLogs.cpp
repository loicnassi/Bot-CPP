//
//  AlgoLogs.cpp
//  Algo
//
//  Created by Loïc Nassi on 29/09/2024.
//

#include "AlgoLogs.hpp"

AlgoLogs::AlgoLogs() {
    
}

AlgoLogs::~AlgoLogs() {
}



//  AlgoLogs function. Forward declaration in the App.hpp file
void AlgoLogs::saveLog(std::string log) {
    std::cout << getDateTime() << " | " << log << std::endl;
}

void AlgoLogs::printLog(std::string log) {
    std::cout << getDateTime() << " | " << log << std::endl;
}

std::string AlgoLogs::getDateTime() {
    // Get the current time point from the system clock
    auto now = std::chrono::system_clock::now();

    // Convert to time_t to extract the time components
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Format the time
    std::tm* localTime = std::localtime(&currentTime);
    
    // Prepare a string stream for the formatted output
    std::ostringstream stream;
    stream << std::put_time(localTime, "%Y-%m-%d %H:%M:%S")
           << '.' << std::setw(3) << std::setfill('0') << millis.count();
    
    return stream.str();
}
