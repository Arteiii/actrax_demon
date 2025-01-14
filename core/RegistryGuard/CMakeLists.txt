cmake_minimum_required(VERSION 3.16)

# RegistryGuard library, linking RegistryHelper
add_library(RegistryGuard STATIC src/RegistryGuard.cpp)
target_link_libraries(RegistryGuard PUBLIC RegistryHelper Logger)
target_include_directories(RegistryGuard PUBLIC include)
