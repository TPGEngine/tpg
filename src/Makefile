# Makefile for tpgengine-tpg

# Build configuration
CMAKE := cmake
BUILD_DIR := build
BUILD_TYPE ?= Debug

.PHONY: all debug release optimized clean test help

# Default target
all: debug

# Debug build
debug:
	@if [ -z "$(TPG)" ]; then \
		echo "ERROR: TPG environment variable is not set"; \
		exit 1; \
	fi
	@cd $(TPG) && $(CMAKE) -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	@cd $(TPG) && $(CMAKE) --build $(BUILD_DIR)

# Release build
release:
	@if [ -z "$(TPG)" ]; then \
		echo "ERROR: TPG environment variable is not set"; \
		exit 1; \
	fi
	@cd $(TPG) && $(CMAKE) -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release
	@cd $(TPG) && $(CMAKE) --build $(BUILD_DIR) --config Release

# Release build with high optimization
optimized:
	@if [ -z "$(TPG)" ]; then \
		echo "ERROR: TPG environment variable is not set"; \
		exit 1; \
	fi
	@cd $(TPG) && $(CMAKE) -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Release -DENABLE_HIGH_OPTIMIZATION=ON
	@cd $(TPG) && $(CMAKE) --build $(BUILD_DIR)

# Clean the build directory
clean:
	@if [ -z "$(TPG)" ]; then \
		echo "ERROR: TPG environment variable is not set"; \
		exit 1; \
	fi
	@if [ -d "$(TPG)/$(BUILD_DIR)" ]; then \
		rm -rf $(TPG)/$(BUILD_DIR)/*; \
		echo "Build directory cleaned"; \
	else \
		echo "Build directory does not exist"; \
	fi

# Run tests
test:
	@if [ -z "$(TPG)" ]; then \
		echo "ERROR: TPG environment variable is not set"; \
		exit 1; \
	fi
	@cd $(TPG) && $(CMAKE) --build $(BUILD_DIR) --target test

# Help information
help:
	@echo "Available targets:"
	@echo "  all        - Build the project in debug mode (default)"
	@echo "  debug      - Configure and build in Debug mode"
	@echo "  release    - Configure and build in Release mode"
	@echo "  optimized  - Configure and build in Release mode with high optimization"
	@echo "  clean      - Clean the build directory"
	@echo "  test       - Run tests"
	@echo ""
	@echo "Note: The TPG environment variable must be set to the root directory of the project."
	@echo "Current TPG: $(TPG)"
	@echo ""
	@echo "Command equivalents (executed from the TPG directory):"
	@echo "  make debug      = cd $(TPG) && cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug"
	@echo "                    cd $(TPG) && cmake --build build"
	@echo ""
	@echo "  make release    = cd $(TPG) && cmake -B build -S . -DCMAKE_BUILD_TYPE=Release"
	@echo "                    cd $(TPG) && cmake --build build --config Release"
	@echo ""
	@echo "  make optimized  = cd $(TPG) && cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DENABLE_HIGH_OPTIMIZATION=ON"
	@echo "                    cd $(TPG) && cmake --build build"
