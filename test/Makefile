# Makefile for testing against C Vibe

# - run ´make´ inside this dir.
# - Requires hardware

# Cross compiler (ARM64)
CXX = aarch64-linux-gnu-g++
CXXFLAGS = -O2 -Wall

TARGET := test
SRC := test.cpp
BUILD_DIR := build
OBJ = $(BUILD_DIR)/test.o

# Default target
clarus: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)