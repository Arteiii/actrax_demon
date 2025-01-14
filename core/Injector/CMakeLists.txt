cmake_minimum_required(VERSION 3.16)

add_library(Injector STATIC src/loadLibrary.cpp)
target_link_libraries(Injector PUBLIC Logger)

target_include_directories(Injector PUBLIC include)

