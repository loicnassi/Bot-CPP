##############################################################################
# Makefile for mathlib static library (mathlib.a)
##############################################################################

# Compiler and Flags
CC := gcc
AR := ar
ARFLAGS := rcs

# Source Directory
SRC_DIR := ./LIBRARY/src

# Flags
CFLAGS := -I$(SRC_DIR) -O2 \
          -DDECIMAL_CALL_BY_REFERENCE=0 -DDECIMAL_GLOBAL_ROUNDING=0 \
          -DDECIMAL_GLOBAL_EXCEPTION_FLAGS=0 -DUNCHANGED_BINARY_FLAGS=0

# Library Name
LIBRARY := mathlib.a

# Source Files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Default Target: Always rebuild
.PHONY: all
all: clean $(LIBRARY)

# Create Static Library Without Intermediary Object Files
$(LIBRARY):
	@$(CC) $(CFLAGS) -c $(SRCS)
	@$(AR) $(ARFLAGS) $@ *.o
	@rm -f *.o

# Clean Target
.PHONY: clean
clean:
	@rm -f $(LIBRARY) *.o
