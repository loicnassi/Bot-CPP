//  Logger.cpp
//  Algo
//
//  Created by Loïc Nassi on 24/01/2025.
//
#include "Logger.hpp"

// Singleton instance
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

// Constructor
Logger::Logger() : isRunning(true), logThread(&Logger::processLogs, this) {}

// Log message asynchronously
void Logger::log(Level level, const std::string& message) {
    std::string logEntry = formatLogEntry(level, message);

    size_t head = queueHead.load(std::memory_order_relaxed);
    size_t next = (head + 1) % QUEUE_SIZE;

    // If the queue is full, drop the message (fastest approach)
    if (next == queueTail.load(std::memory_order_acquire)) {
        return;
    }

    logQueue[head] = std::move(logEntry);
    queueHead.store(next, std::memory_order_release);
    
    cv.notify_one();
}

// Set log file and enable rotation
void Logger::setLogFile(const std::string& filename, size_t maxSize) {
    logFilename = filename;
    maxFileSize = maxSize;
    rotateLogFile();
}

// Enable or disable console logging
void Logger::enableConsoleLogging(bool enable) {
    consoleLogging = enable;
}

// Log processing thread
void Logger::processLogs() {
    while (isRunning) {
        std::string logBatch;
        
        size_t tail = queueTail.load(std::memory_order_acquire);
        while (tail != queueHead.load(std::memory_order_acquire)) {
            logBatch += logQueue[tail] + "\n";
            tail = (tail + 1) % QUEUE_SIZE;
        }
        
        if (!logBatch.empty()) {
            writeToFile(logBatch);
            if (consoleLogging) {
                std::cout << logBatch;
            }
        }

        queueTail.store(tail, std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1)); // Avoid busy-waiting
    }
}

// Write logs to file efficiently
void Logger::writeToFile(const std::string& logBatch) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!logFile.is_open() || logFile.tellp() >= static_cast<std::streampos>(maxFileSize)) {
        rotateLogFile();
    }
    if (logFile.is_open()) {
        logFile << logBatch;
        logFile.flush();
    }
}

// Rotate log files when size exceeds limit
void Logger::rotateLogFile() {
    if (logFile.is_open()) {
        logFile.close();
    }

    std::ostringstream newFileName;
    newFileName << logFilename << "_" << currentTimestamp() << ".log";
    logFile.open(newFileName.str(), std::ios::app);
}

// Get current timestamp
std::string Logger::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now_c));
    return std::string(buffer);
}

// Convert log level to string
std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        case Level::DETAIL: return "DETAIL";
        default: return "UNKNOWN";
    }
}

// Stop logging and cleanup
void Logger::stopLogging() {
    isRunning = false;
    cv.notify_one();
    if (logThread.joinable()) {
        logThread.join();
    }
}

// Destructor
Logger::~Logger() {
    stopLogging();
}
