# Compiler and flags
CXX = clang++
CXXFLAGS = -std=c++20 -I./client -I.

# Target executable
TARGET = AlgoBot

# Directories
ALGO_DIR = ./Algo
CLIENT_DIR = ./client
MATHLIB_DIR = ./mathlib

# Static library
LIBS = $(MATHLIB_DIR)/mathlib.a

ALGO_SRCS = $(shell find $(ALGO_DIR) -name '*.cpp')
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.cpp)

# Combine all source files
SRC = $(ALGO_SRCS) $(CLIENT_SRCS)

# Build target
$(TARGET): $(SRC) $(LIBS)
	@echo "Compiling and linking $(TARGET)..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# Ensure mathlib.a is always rebuilt
.PHONY: $(LIBS)
$(LIBS):
	@echo "Building mathlib.a..."
	@$(MAKE) -C $(MATHLIB_DIR) clean
	@$(MAKE) -C $(MATHLIB_DIR)

# Clean target
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)
	$(MAKE) -C $(MATHLIB_DIR) clean
