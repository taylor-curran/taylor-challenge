# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17

# Directories
SRC_DIR := emulator
BUILD_DIR := .
DATA_DIR := data
TEST_DIR := tests
CMAKE_BUILD_DIR := build

# Source files
SRCS := $(SRC_DIR)/rover_emulator.cpp
HDRS := $(SRC_DIR)/rover_profiles.h
TARGET := $(BUILD_DIR)/rover_emulator

# Test files
TEST_SRCS := $(TEST_DIR)/test_udp_listener.cpp
TEST_TARGET := $(TEST_DIR)/test_udp_listener

# Default rule: build the emulator
.PHONY: all
all: $(TARGET) extract

# Compile the main executable
$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

# Build test targets
.PHONY: test
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_TARGET)

# Extract .dat files only if they don't already exist
.PHONY: extract
extract:
	@for archive in data/*.tar.xz; do \
		extracted_file=$$(basename $$archive .tar.xz); \
		if [ ! -f "data/$$extracted_file" ]; then \
			echo "Extracting $$archive..."; \
			tar -xJf $$archive -C data/ --strip-components=1; \
		else \
			echo "$$extracted_file already exists. Skipping extraction."; \
		fi \
	done

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)

# Runs all rover emulators
.PHONY: run
run: extract
	./run_rovers.sh

.PHONY: run-noiseless
run-noiseless: extract
	./run_rovers.sh --no-noise

# ===== CMake Build Commands =====
.PHONY: cmake-config
cmake-config:
	@echo "Configuring CMake project..."
	@cmake -S . -B $(CMAKE_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

.PHONY: cmake-build
cmake-build: cmake-config
	@echo "Building with CMake..."
	@cmake --build $(CMAKE_BUILD_DIR) -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

.PHONY: cmake-clean
cmake-clean:
	@echo "Cleaning CMake build..."
	@rm -rf $(CMAKE_BUILD_DIR)

.PHONY: build-viz
build-viz: cmake-build
	@echo "Visualization app built: $(CMAKE_BUILD_DIR)/bin/lidar_viz"
