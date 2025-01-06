//
//  App.cpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#include "App.hpp"


App::App(const char *host, int port, int clientId):
    
//  Class Attributes 
    EClientSocket(this, &signal),
    signal(2000),
    storedBasketStrategy(nullptr),
    storedPortfolio(nullptr),
    logs(),
    
//  App Attribute Management
    orderId(0),
    requestId(0),

    retrieved(false),
    connected(false),
    end(false)
,
    requests() {
        
    bool conn = eConnect(host, port, clientId, false);
        
    if (conn) {
        reader = new EReader(this, &signal);
        reader->start();
        std::cout<<"Connected"<<std::endl;
    }
    else
        std::cout << "Failed to connect" << std::endl;
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
    printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %s, %s, %d\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, decimalStringToDisplay(execution.shares).c_str(), decimalStringToDisplay(execution.cumQty).c_str(), execution.lastLiquidity);
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

void App::error(int id, int code, const std::string& msg, const std::string& advancedOrderRejectJson) {
    
    connected = true;
    std::cout << "Error: " << code << ": " << msg << std::endl;
}


