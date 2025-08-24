# Default compiler and flags
CXX := $(TOOLCHAIN_PREFIX)g++

# NOTE: This Makefile is designed only to run on my own machine. Sorry.
INCLUDES := -I. -I/opt/homebrew/include $(INCLUDES)

EXEC := SignatureScannerTest

# Default target
all: $(EXEC)

# Linking the executable -fsanitize=address,undefined
$(EXEC):
	$(CXX) -std=c++20 -g -Wall -Wextra -Wconversion \
	$(INCLUDES) -lgtest -lgtest_main \
	../src/utils/SignatureScanner.cpp SignatureScannerTest.cpp ../src/patches.cpp -o $(EXEC)
