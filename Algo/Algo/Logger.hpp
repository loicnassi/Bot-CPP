//
//  Logger.hpp
//  Algo
//
//  Created by Loïc Nassi on 24/01/2025.
//

#ifndef Logger_hpp
#define Logger_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>
#include <sstream>
#include <chrono>
#include <ctime>

#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>
#include <sstream>

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>

constexpr size_t QUEUE_SIZE = 1024;  // Lock-free queue capacity

class Logger {
public:
    enum class Level { INFO, WARNING, ERROR, DETAIL };

    static Logger& getInstance();

    void log(Level level, const std::string& message);
    void setLogFile(const std::string& filename, size_t maxSize = 1024 * 1024);
    void enableConsoleLogging(bool enable);

    ~Logger();

private:
    std::string logQueue[QUEUE_SIZE];
    std::atomic<size_t> queueHead{0};
    std::atomic<size_t> queueTail{0};

    std::thread logThread;
    std::atomic<bool> isRunning;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::ofstream logFile;
    std::string logFilename = "log.txt";
    size_t maxFileSize = 1024 * 1024;
    bool consoleLogging = false;

    Logger();
    void processLogs();
    void writeToFile(const std::string& logBatch);
    void rotateLogFile();
    std::string formatLogEntry(Level level, const std::string& message);
    std::string currentTimestamp();
    std::string levelToString(Level level);
    void stopLogging();
};


#endif /* Logger_hpp */
