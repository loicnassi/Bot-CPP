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
    logs(),

    // App Attribute Management
    orderId(0),
    requestId(0),

    retrieved(false),
    connected(false),
    end(false),
    requests() {

    try {
        // Log the start of initialization
        std::cout << "Initializing App..." << std::endl;
        std::cout << "Host: " << host << ", Port: " << port << ", Client ID: " << clientId << std::endl;

        // Start connection attempt
        auto start = std::chrono::steady_clock::now();
        bool conn = eConnect(host, port, clientId, false);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (conn) {
            std::cout << "Connection established successfully to " << host << ":" << port
                      << " in " << duration << " ms" << std::endl;

            // Start the reader thread and log its state
            reader = new EReader(this, &signal);
            reader->start();
            std::cout << "EReader thread started successfully." << std::endl;

        } else {
            std::cerr << "Connection failed to " << host << ":" << port
                      << ". Error: Failed to establish socket connection." << std::endl;
            throw std::runtime_error("Connection failed.");
        }
    } catch (const std::exception &ex) {
        std::cerr << "Exception caught during connection: " << ex.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred during connection initialization." << std::endl;
    }
}

App::~App() { delete reader;
}

//  Used functions
void App::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close, Decimal volume, Decimal wap, int count){
    
    Bar bar;
    bar.time = std::to_string(time);
    bar.open = open;
    bar.high = high;
    bar.low = low;
    bar.close = close;
    bar.volume = volume;
    bar.count = count;
    bar.wap = wap;
    
    if (storedBasketStrategy) {
        handlingBar(this->requests[reqId], bar);
    }
}

void App::wait(std::string type) {
    
    bool* condition = nullptr;
    
    if (type == "connect") {
        condition = &connected;
    }
    else if (type == "retrieve") {
        condition = &retrieved;
    }
    else {
        condition = &end;
    }
    
    while (!(*condition)) {
        signal.waitForSignal();
        reader->processMsgs();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    };
    *condition = false;
}

void App::accountSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
    
    if (storedPortfolio) {
        handlingSummary(reqId, account, tag, value, currency);
    }
}

void App::accountSummaryEnd(int reqId) {

    this->cancelAccountSummary(reqId);
    retrieved = true;
}

void App::execDetails(int reqId, const Contract &contract, const Execution &execution) {
    
//    if (storedPortfolio) {
//        handlingOrders(reqId, contract, execution);
//    }
//    
//    printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %s, %s, %d\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, DecimalFunctions::decimalStringToDisplay(execution.shares).c_str(), DecimalFunctions::decimalStringToDisplay(execution.cumQty).c_str(), execution.lastLiquidity);
}

void App::execDetailsEnd(int reqId) {
    
}

void App::commissionReport(const CommissionReport &commissionReport) {
    
//    printf( "CommissionReport. %s - %f %s RPNL %f\n", commissionReport.execId.c_str(), commissionReport.commission, commissionReport.currency.c_str(), commissionReport.realizedPNL);
}

void App::nextValidId(OrderId orderId) {
    
    this->orderId = orderId;
    printf("Next ID : %ld\n", orderId);
}

void App::error(int id, time_t errorTime, int errorCode,const std::string& errorString, const std::string& advancedOrderRejectJson) {
    
    connected = true;
    std::cout << "Error: " << errorCode << ": " << errorString << std::endl;
}


