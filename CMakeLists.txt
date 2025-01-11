cmake_minimum_required(VERSION 3.20)

project(FileParser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME}
    Ini.cpp
    Json.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ./)
