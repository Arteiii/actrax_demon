cmake_minimum_required(VERSION 3.16)

add_library(Integrity STATIC src/AntiAnalysis.cpp
        include/StringEncryptionUtils.hpp)

target_link_libraries(RegistryGuard PUBLIC kernel32)

target_include_directories(Integrity INTERFACE include)
