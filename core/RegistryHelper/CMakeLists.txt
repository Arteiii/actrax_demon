cmake_minimum_required(VERSION 3.16)

# RegistryHelper library
add_library(RegistryHelper STATIC src/RegistryHelper.cpp)
target_include_directories(RegistryHelper PUBLIC include)
