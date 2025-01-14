cmake_minimum_required(VERSION 3.16)


add_library(Server STATIC
        src/Session.cpp
        include/SessionToken.hpp
        src/Connect.cpp
        include/Helper.hpp
        include/Client.hpp
        include/Tasks.hpp)

target_link_libraries(Server PUBLIC Logger nlohmann_json::nlohmann_json Integrity winhttp
)

target_include_directories(Server PUBLIC include)
