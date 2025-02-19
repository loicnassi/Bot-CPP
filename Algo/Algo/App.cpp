//
//  App.cpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#include "App.hpp"


App::App(const char *host, int port, int clientId) :
    // Class Attributes
    EClientSocket(this, &signal),
    signal(2000),
    storedBasketStrategy(nullptr),
    storedPortfolio(nullptr),

    // App Attribute Management
    orderId(0),
    requestId(0),

    connected(false),
    end(false),
    retrieved(),
    requests() {
        
    try {
        Logger::getInstance().log(Logger::Level::INFO, std::format("Host: {}, Port: {}, Client ID: {}", host, port, clientId));
        // Start connection attempt
        bool conn = eConnect(host, port, clientId, false);

        if (conn) {
            // Start the reader thread and log its state
            reader = new EReader(this, &signal);
            reader->start();
            Logger::getInstance().log(Logger::Level::INFO, "EReader thread started successfully.");
            }
        else {
            Logger::getInstance().log(Logger::Level::ERROR, std::format("Connection failed to {}:{}", host, port));
        }
    } catch (const std::exception &ex) {
        Logger::getInstance().log(Logger::Level::ERROR, std::format("Exception caught during connection: {}", ex.what()));
    } catch (...) {
        Logger::getInstance().log(Logger::Level::ERROR, "Unknown error occurred during connection initialization.");
    }
}

App::~App() { delete reader;
}

//  Request Management
void App::wait(std::string type, int reqId) {
    
    bool* condition = nullptr;
    
    if (type == "connect") {
        condition = &connected;
    }
    else if (type == "retrieve") {
        condition = &retrieved[reqId];
    }
    else {
        condition = &end;
    }
    
    while (!(*condition)) {
        signal.waitForSignal();
        reader->processMsgs();
    }
    
    if (type == "retrieve") {
        retrieved.erase(reqId);
    }
    else {
        *condition = false;
    }
}

double App::incrementRequestId() {
    
    std::lock_guard<std::mutex> lock(requestMutex);
    requestId++;
    std::string threadId = Logger::getInstance().formatThreadId(std::this_thread::get_id());
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("RequestId : {} | Thread: {}", requestId, threadId));
    
    return requestId;
}

double App::incrementOrderId() {
    
    std::lock_guard<std::mutex> lock(orderMutex);
    orderId++;
    std::string threadId = Logger::getInstance().formatThreadId(std::this_thread::get_id());
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("OrderId : {} | Thread: {}", orderId, threadId));
    
    return orderId;
}

//  Used functions

void App::nextValidId(OrderId orderId) {
    
    connected = true;
    this->orderId = orderId;
    Logger::getInstance().log(Logger::Level::INFO, std::format("Next ID: {}", orderId));
}

void App::error(int id, time_t errorTime, int errorCode,const std::string& errorString, const std::string& advancedOrderRejectJson) {
    
    Logger::getInstance().log(Logger::Level::INFO, std::format("Error Code {}: {}", errorCode, errorString));
}

void App::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, Decimal volume, Decimal wap, int count) {
    
    Bar bar;
    bar.time = std::to_string(time);
    bar.open = open;
    bar.high = high;
    bar.low = low;
    bar.close = close;
    bar.volume = volume;
    bar.count = count;
    bar.wap = wap;
    
    if (storedBasketStrategy) handleBar(reqId, this->requests[reqId], bar);
}

void App::historicalData(TickerId reqId, const Bar &bar) {
    
    if (storedBasketStrategy) {
        fitBar(reqId, this->requests[reqId], bar, false);
    }
}

void App::historicalDataUpdate(TickerId reqId, const Bar &bar) {
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("ReqId: {} | Update Price", reqId));
    if (storedBasketStrategy) {
        fitBar(reqId, this->requests[reqId], bar, true);
    }
}

void App::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("ReqId: {} | End of Historical Data | Start date: {} | End Date: {}", reqId, startDateStr, endDateStr));
}

void App::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
    
    if (storedPortfolio) {
        handleSummary(reqId, account, tag, value, currency);
    }
}

void App::accountSummaryEnd(int reqId) {

    this->cancelAccountSummary(reqId);
    retrieved[reqId]=true;
}

void App::execDetails(int reqId, const Contract &contract, const Execution &execution) {
    
    if (storedPortfolio) {
        handleOrders(reqId, contract, execution);
    }
    printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %f, %s, %s, %d\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, execution.avgPrice, DecimalFunctions::decimalStringToDisplay(execution.shares).c_str(), DecimalFunctions::decimalStringToDisplay(execution.cumQty).c_str(), execution.lastLiquidity);
}

void App::execDetailsEnd(int reqId) {
    
}

void App::commissionReport(const CommissionReport &commissionReport) {
    
    printf( "CommissionReport. %s - %f %s RPNL %f\n", commissionReport.execId.c_str(), commissionReport.commission, commissionReport.currency.c_str(), commissionReport.realizedPNL);
}

void App::position(const std::string& account, const Contract& contract, Decimal position, double avgCost) {
    
    if (storedPortfolio) {
        handlePositions(contract, position, avgCost);
    }
    Logger::getInstance().log(Logger::Level::DETAIL, std::format("Account: {} | Contract: {} | Position: {} | Average Cost {}", account, contract.symbol, DecimalFunctions::decimalStringToDisplay(position), avgCost));
    
}

void App::positionEnd() {
    
    cancelPositions();
}
