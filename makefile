# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++20 -I./client -I.

# Target executable
TARGET = AlgoBot

# Source files (explicitly listed based on Xcode)
SRC = Algo/Algo/main.cpp \
      Algo/Algo/AlgoLogs.cpp \
      Algo/Algo/App.cpp \
      Algo/Algo/Asset.cpp \
      Algo/Algo/Portfolio.cpp \
      Algo/Algo/Strategies/BasketTrading/Basket.cpp \
      Algo/Algo/Strategies/BasketTrading/BasketTradingBot.cpp \
      client/ContractCondition.cpp \
      client/DefaultEWrapper.cpp \
      client/EClient.cpp \
      client/EClientSocket.cpp \
      client/EDecoder.cpp \
      client/EMessage.cpp \
      client/EMutex.cpp \
      client/EOrderDecoder.cpp \
      client/EReader.cpp \
      client/EReaderOSSignal.cpp \
      client/ESocket.cpp \
      client/MarginCondition.cpp \
      client/OperatorCondition.cpp \
      client/OrderCondition.cpp \
      client/PercentChangeCondition.cpp \
      client/PriceCondition.cpp \
      client/SoftDollarTier.cpp \
      client/StdAfx.cpp \
      client/TimeCondition.cpp \
      client/Utils.cpp \
      client/VolumeCondition.cpp \
      client/executioncondition.cpp

# Static library
LIBS = client/lib/mathlib_arm.a

# Build and run target
run: $(TARGET)
	./$(TARGET) # This runs the application after building

# Build target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Clean target
clean:
	rm -f $(TARGET)
