cmake_minimum_required(VERSION 3.16)


add_library(TaskScheduler STATIC
        src/TaskScheduler.cpp)

target_link_libraries(TaskScheduler PUBLIC Logger)

target_include_directories(TaskScheduler PUBLIC include)
