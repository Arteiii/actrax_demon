BUILD_DIR = build

CMAKE_GENERATOR = "Unix Makefiles"
CMAKE_OPTIONS = -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

TOOLCHAIN_DIR = ./cmake/toolchains
TOOLCHAIN_MINGW = $(TOOLCHAIN_DIR)/mingw.cmake

# Default (mingw)
COMPILER ?= mingw
TOOLCHAIN = $(TOOLCHAIN_MINGW)


# Default target
all: release

# Configures and builds for Release
configure_release:
	@echo "Configuring project for Release with $(COMPILER)..."
	@mkdir -p $(BUILD_DIR)/release
	@cd $(BUILD_DIR)/release && cmake -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) $(CMAKE_OPTIONS) ../..

build_release:
	@echo "Building project for Release with $(COMPILER)..."
	@cd $(BUILD_DIR)/release && make

# Configures and builds for Debug
configure_debug:
	@echo "Configuring project for Debug with $(COMPILER)..."
	@mkdir -p $(BUILD_DIR)/debug
	@cd $(BUILD_DIR)/debug && cmake -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) $(CMAKE_OPTIONS) ../..

build_debug:
	@echo "Building project for Debug with $(COMPILER)..."
	@cd $(BUILD_DIR)/debug && make

# Configures and builds for Release with Debug Info
configure_release_debug_info:
	@echo "Configuring project for Release with Debug Info with $(COMPILER)..."
	@mkdir -p $(BUILD_DIR)/RelWithDebInfo
	@cd $(BUILD_DIR)/RelWithDebInfo && cmake -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) $(CMAKE_OPTIONS) ../..

build_release_debug_info:
	@echo "Building project for Release with Debug Info with $(COMPILER)..."
	@cd $(BUILD_DIR)/RelWithDebInfo && make

# Configures and builds for MinSizeRel
configure_minsize_release:
	@echo "Configuring project for Min Size Release with $(COMPILER)..."
	@mkdir -p $(BUILD_DIR)/MinSizeRel
	@cd $(BUILD_DIR)/MinSizeRel && cmake -G $(CMAKE_GENERATOR) -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) $(CMAKE_OPTIONS) ../..

build_minsize_release:
	@echo "Building project for Min Size Release with $(COMPILER)..."
	@cd $(BUILD_DIR)/MinSizeRel && make

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Rebuild targets
rebuild_release: clean configure_release build_release
rebuild_debug: clean configure_debug build_debug

# Serve debug output via HTTP server on port 4444
serve: debug
	@cd $(BUILD_DIR) && python3 -m http.server 4444

.PHONY: all configure_release build_release configure_debug build_debug clean rebuild_release rebuild_debug release debug

# Shortcut targets
release: configure_release build_release
debug: configure_debug build_debug
release_debug: configure_release_debug_info build_release_debug_info
minsize_release: configure_minsize_release build_minsize_release
