# Default compiler and flags
CXX := $(TOOLCHAIN_PREFIX)g++

# NOTE: This Makefile is designed only to run on my own machine. Sorry.
INCLUDES := -I. -I/opt/homebrew/include $(INCLUDES)

# Default target
all: SignatureFFLMatchTest # SignatureScannerTest

# Linking the executable -fsanitize=address,undefined
SignatureScannerTest:
	$(CXX) -std=c++20 -g -Wall -Wextra -Wconversion \
	$(INCLUDES) -lgtest -lgtest_main \
	../src/utils/SignatureScanner.cpp SignatureScannerTest.cpp -o SignatureScannerTest

SignatureFFLMatchTest:
	$(CXX) -std=c++20 -g -Wall -Wextra -Wconversion \
	$(INCLUDES) -lgtest -lgtest_main \
	../src/utils/SignatureScanner.cpp SignatureFFLMatchTest.cpp ../src/ffl_patches.cpp -o SignatureFFLMatchTest
