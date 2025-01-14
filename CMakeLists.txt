cmake_minimum_required(VERSION 3.16)
project(ActraxDemon VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_language(CXX)

include(FetchContent)

message(STATUS "Configuring project: ${PROJECT_NAME} version ${PROJECT_VERSION}")

set(TARGET_BITNESS "x64" CACHE STRING "Target architecture (x86 or x64)")
message(STATUS "Target bitness: ${TARGET_BITNESS}")

if (TARGET_BITNESS STREQUAL "x64")
    set(OSBitness 64)
    add_compile_definitions(X64_COMPILE)
    message(STATUS "Detected 64-bit architecture.")
elseif (TARGET_BITNESS STREQUAL "x86")
    set(OSBitness 32)
    message(STATUS "Detected 32-bit architecture.")
else ()
    message(FATAL_ERROR "Invalid TARGET_BITNESS value. Use 'x86' or 'x64'.")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
message(STATUS "Setting CMake CXX flags: ${CMAKE_CXX_FLAGS}")

if (CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32-g\\+\\+|i686-w64-mingw32-g\\+\\+")
    set(PlatformName "WinGCC")
    set(CMAKE_RC_COMPILER "x86_64-w64-mingw32-windres")
    message(STATUS "Detected MinGW-GCC toolchain.")
else ()
    set(PlatformName "Linux")
    message(STATUS "Compiling for Linux detected.")
endif ()

if (PlatformName STREQUAL "WinGCC")
    set(WIN_SDK_INCLUDE_DIR "/usr/share/mingw-w64/include")
    set(WIN_SDK_LIB_DIR "/usr/share/mingw-w64/lib")
    include_directories(${WIN_SDK_INCLUDE_DIR})
    link_directories(${WIN_SDK_LIB_DIR})
    message(STATUS "Added Windows SDK include and library paths.")
endif ()

if (NOT CMAKE_CXX_COMPILER MATCHES "x86_64-w64-mingw32-g\\+\\+" AND NOT CMAKE_CXX_COMPILER MATCHES "i686-w64-mingw32-g\\+\\+")
    message(FATAL_ERROR "MinGW toolchain not detected. Please install it by running: 'sudo apt install mingw-w64'. More details: https://wiki.debian.org/Mingw")
endif ()

if (PlatformName STREQUAL "Linux")
    find_package(BCRYPT REQUIRED)
    if (NOT BCRYPT_FOUND)
        message(FATAL_ERROR "Bcrypt library not found. Please install it by running: 'sudo apt install libbcrypt-dev'.")
    endif ()

    find_package(IPHLPAPI REQUIRED)
    if (NOT IPHLPAPI_FOUND)
        message(FATAL_ERROR "Iphlpapi library not found. Please install it by running: 'sudo apt install libiphlpapi-dev'.")
    endif ()

    find_package(KERNEL32 REQUIRED)
    if (NOT KERNEL32_FOUND)
        message(FATAL_ERROR "Kernel32 library not found. Please install it by running: 'sudo apt install libkernel32-dev'.")
    endif ()

    find_package(USER32 REQUIRED)
    if (NOT USER32_FOUND)
        message(FATAL_ERROR "User32 library not found. Please install it by running: 'sudo apt install libuser32-dev'.")
    endif ()

    find_package(ADVAPI32 REQUIRED)
    if (NOT ADVAPI32_FOUND)
        message(FATAL_ERROR "Advapi32 library not found. Please install it by running: 'sudo apt install libadvapi32-dev'.")
    endif ()

    find_package(GDI32 REQUIRED)
    if (NOT GDI32_FOUND)
        message(FATAL_ERROR "Gdi32 library not found. Please install it by running: 'sudo apt install libgdi32-dev'.")
    endif ()
endif ()


# security flags
if (PlatformName STREQUAL "WinGCC")
    add_compile_options(
            -fcf-protection=full          # Control Flow Guard (CFG)
            -DYNAMICBASE                  # ASLR
            -Wl,--nxcompat                # DEP in the linker
            -Wl,--dynamicbase             # ASLR in the linker
            -Wl,--high-entropy-va         # 64-bit ASLR
    )
    message(STATUS "Enabled security-related compiler flags.")
endif ()

set(FullOutputDir "${CMAKE_SOURCE_DIR}/bin/${PROJECT_VERSION}/${PlatformName}${OSBitness}/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${FullOutputDir}/static_libs")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${FullOutputDir}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")


message(STATUS "Output directory set to: ${FullOutputDir}")

message(STATUS "CMake C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMake C++ Standard Required: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMake C++ Extensions: ${CMAKE_CXX_EXTENSIONS}")
message(STATUS "CMake CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMake CXX Debug Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMake CXX Release Flags: ${CMAKE_CXX_FLAGS_RELEASE}")


if (NOT json_POPULATED)
    FetchContent_Declare(json
            GIT_REPOSITORY https://github.com/nlohmann/json
            GIT_TAG v3.11.3
    )

    FetchContent_MakeAvailable(json)
else ()
    message(STATUS "json dependency already available.")
endif ()


# Add subdirectories
add_subdirectory(core)
include_directories(.)

set(SOURCES
        src/main.cpp
        src/modules/RegistryManager/RegistryManager.cpp
        src/modules/RegistryManager/RegistryManager.hpp
        src/modules/ApplicationController/ApplicationController.cpp
        src/modules/ApplicationController/ApplicationController.hpp
        src/modules/System/SystemInfo.cpp
        src/modules/System/SystemInfo.hpp
        src/modules/System/SystemManager.cpp
        src/modules/System/SystemManager.hpp
        src/modules/Utility/Utility.cpp
        src/modules/Utility/Utility.hpp
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND SOURCES
            resources_debug.rc
    )
else ()
    list(APPEND SOURCES
            resources.rc
    )
endif ()

add_executable(${PROJECT_NAME} ${SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            COMPILE_TIME_MINIMAL_OUTPUT=1
            COMPILE_TIME_MIN_LOG_LEVEL=log_level::Trace
    )
    message(STATUS "Debug build: Logging level set to Trace.")
    # GCC-specific debug settings (DWARF)
    add_compile_options(-g -gdwarf-4)
    message(STATUS "Debug mode with DWARF debug info enabled for GCC.")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            COMPILE_TIME_MINIMAL_OUTPUT=1
            COMPILE_TIME_MIN_LOG_LEVEL=log_level::Info
    )
    message(STATUS "Release build: Logging level set to INFO.")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -O3 -fomit-frame-pointer")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0 -O3 -fomit-frame-pointer")
    message(STATUS "Release mode with optimizations and no debug info.")

    set_target_properties(${PROJECT_NAME} PROPERTIES
            POST_BUILD_COMMAND "${CMAKE_STRIP} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROJECT_NAME}"
    )

elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            COMPILE_TIME_MINIMAL_OUTPUT=1
            COMPILE_TIME_MIN_LOG_LEVEL=log_level::None
    )
    message(STATUS "MinSizeRel build: Logging level set to None.")

    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -g0 -O3 -fomit-frame-pointer")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -g0 -O3 -fomit-frame-pointer")
    message(STATUS "MinSizeRel mode with no debug info and optimizations enabled.")

    set_target_properties(${PROJECT_NAME} PROPERTIES
            POST_BUILD_COMMAND "${CMAKE_STRIP} ${PROJECT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROJECT_NAME}"
    )

else ()
    message(STATUS "Unknown or no build type: Logging level set to default.")
endif ()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Logger
        TaskScheduler
        RegistryGuard
        Server
        Integrity
        Injector

        # Windows libraries:
        bcrypt
        iphlpapi
        kernel32
        user32

        # nlohmann/json
        nlohmann_json::nlohmann_json

        ${CMAKE_CRT_LIBRARY}
)

message(STATUS "Adding main executable target: ${PROJECT_NAME}")
message(STATUS "Linking libraries to target: ${PROJECT_NAME}")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(${CMAKE_SOURCE_DIR}/zip-hash.cmake)
endif ()

