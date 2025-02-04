//
//  App.hpp
//  Algo
//
//  Created by Loïc Nassi on 25/09/2023.
//

#ifndef App_hpp
#define App_hpp

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <future>
#include <unordered_map>
#include <variant>
#include <atomic>
#include <mutex>
#include <chrono>
#include <stdexcept>

#include "Logger.hpp"

#include "EReader.h"
#include "EReaderOSSignal.h"
#include "EWrapper.h"
#include "EClientSocket.h"
#include "Utils.h"
#include "Order.h"
#include "Execution.h"
#include "CommissionReport.h"


//Forward declaration
class Portfolio;
class BasketTradingBot;
class Basket;
class Asset;

class App : public EWrapper, public EClientSocket {
    
public:
    
    App(const char *host, int port, int clientId = 0);
    ~App();
    
//  Major features
    EReader *reader;
    EReaderOSSignal signal;
    BasketTradingBot *storedBasketStrategy;
    Portfolio *storedPortfolio;

//  App Attribute Management
    double orderId;
    double requestId;
    std::mutex requestMutex;
    std::mutex orderMutex;
    
    bool connected;
    bool end;
    
    std::unordered_map<double, bool> retrieved;
    std::unordered_map<double, Asset*> requests;
    
//  Created void functions
    void wait(std::string type = "end", int reqId = 0);
    double incrementRequestId();
    double incrementOrderId();
    void handleBar(long reqId, Asset* asset, Bar bar); // Forward declaration, function declared in each Strategy
    void fitBar(long reqId, Asset* asset, Bar bar, bool update); // Forward declaration, function declared in each Strategy
    void handleSummary(int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency); // Forward declaration, function has to be declared in Portfolio
    void handleOrders(int reqId, const Contract &contract, const Execution &execution);
    void handlePositions(const Contract &contract, Decimal position, double avgCost);
    void fetchAsset(const Contract &contract, std::string side, double quantity);
    
//  Used functions
    void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                     Decimal volume, Decimal wap, int count);
    void historicalData(TickerId reqId, const Bar& bar);
    void historicalDataUpdate(TickerId reqId, const Bar& bar);
    void historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr);
    void accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& curency);
    void accountSummaryEnd( int reqId);
    void position(const std::string& account, const Contract& contract, Decimal position, double avgCost);
    void execDetails( int reqId, const Contract& contract, const Execution& execution);
    void execDetailsEnd( int reqId);
    void commissionReport( const CommissionReport& commissionReport);
    void nextValidId(OrderId orderId);
    void error(int id, time_t errorTime, int errorCode,const std::string& errorString, const std::string& advancedOrderRejectJson);

    
//  Non used functions
    virtual void tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attrib) {};
    virtual void tickSize(TickerId tickerId, TickType field, Decimal size) {};
    virtual void tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
        double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice) {};
    virtual void tickGeneric(TickerId tickerId, TickType tickType, double value) {};
    virtual void tickString(TickerId tickerId, TickType tickType, const std::string& value) {};
    virtual void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
        double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {};
    virtual void orderStatus( OrderId orderId, const std::string& status, Decimal filled, Decimal remaining, double avgFillPrice, long long permId, int parentId, double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {};
    virtual void openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&) {};
    virtual void openOrderEnd() {};
    virtual void winError( const std::string& str, int lastError) {};
    virtual void connectionClosed() {};
    virtual void updateAccountValue(const std::string& key, const std::string& val,
    const std::string& currency, const std::string& accountName) {};
    virtual void updatePortfolio( const Contract& contract, Decimal position,
        double marketPrice, double marketValue, double averageCost,
        double unrealizedPNL, double realizedPNL, const std::string& accountName) {};
    virtual void updateAccountTime(const std::string& timeStamp) {};
    virtual void accountDownloadEnd(const std::string& accountName) {};
    virtual void contractDetails( int reqId, const ContractDetails& contractDetails) {};
    virtual void bondContractDetails( int reqId, const ContractDetails& contractDetails) {};
    virtual void contractDetailsEnd( int reqId) {};
    virtual void updateMktDepth(TickerId id, int position, int operation, int side,
        double price, Decimal size) {};
    virtual void updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
        int side, double price, Decimal size, bool isSmartDepth) {};
    virtual void updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {};
    virtual void managedAccounts( const std::string& accountsList) {};
    virtual void receiveFA(faDataType pFaDataType, const std::string& cxml) {};
    virtual void scannerParameters(const std::string& xml) {};
    virtual void scannerData(int reqId, int rank, const ContractDetails& contractDetails,
        const std::string& distance, const std::string& benchmark, const std::string& projection,
        const std::string& legsStr) {};
    virtual void scannerDataEnd(int reqId) {};
    virtual void currentTime(long time) {};
    virtual void fundamentalData(TickerId reqId, const std::string& data) {};
    virtual void deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {};
    virtual void tickSnapshotEnd( int reqId) {};
    virtual void marketDataType( TickerId reqId, int marketDataType) {};
    virtual void commissionAndFeesReport( const CommissionAndFeesReport& commissionAndFeesReport) {};
    virtual void positionEnd() {};
    virtual void verifyMessageAPI( const std::string& apiData) {};
    virtual void verifyCompleted( bool isSuccessful, const std::string& errorText) {};
    virtual void displayGroupList( int reqId, const std::string& groups) {};
    virtual void displayGroupUpdated( int reqId, const std::string& contractInfo) {};
    virtual void verifyAndAuthMessageAPI( const std::string& apiData, const std::string& xyzChallange) {};
    virtual void verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {};
    virtual void connectAck() {};
    virtual void positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) {};
    virtual void positionMultiEnd( int reqId) {};
    virtual void accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {};
    virtual void accountUpdateMultiEnd( int reqId) {};
    virtual void securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
        const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {};
    virtual void securityDefinitionOptionalParameterEnd(int reqId) {};
    virtual void softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {};
    virtual void familyCodes(const std::vector<FamilyCode> &familyCodes) {};
    virtual void symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {};
    virtual void mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {};
    virtual void tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {};
    virtual void smartComponents(int reqId, const SmartComponentsMap& theMap) {};
    virtual void tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {};
    virtual void newsProviders(const std::vector<NewsProvider> &newsProviders) {};
    virtual void newsArticle(int requestId, int articleType, const std::string& articleText) {};
    virtual void historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {};
    virtual void historicalNewsEnd(int requestId, bool hasMore) {};
    virtual void headTimestamp(int reqId, const std::string& headTimestamp) {};
    virtual void histogramData(int reqId, const HistogramDataVector& data) {};
    virtual void rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {};
    virtual void rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {};
    virtual void marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {};
    virtual void pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {};
    virtual void pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {};
    virtual void historicalTicks(int reqId, const std::vector<HistoricalTick> &ticks, bool done) {};
    virtual void historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk> &ticks, bool done) {};
    virtual void historicalTicksLast(int reqId, const std::vector<HistoricalTickLast> &ticks, bool done) {};
    virtual void tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {};
    virtual void tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) {};
    virtual void tickByTickMidPoint(int reqId, time_t time, double midPoint) {};
    virtual void orderBound(long long orderId, int apiClientId, int apiOrderId) {};
    virtual void completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {};
    virtual void completedOrdersEnd() {};
    virtual void replaceFAEnd(int reqId, const std::string& text) {};
    virtual void wshMetaData(int reqId, const std::string& dataJson) {};
    virtual void wshEventData(int reqId, const std::string& dataJson) {};
    virtual void historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) {};
    virtual void userInfo(int reqId, const std::string& whiteBrandingId) {};
};

#endif /* App_hpp */
