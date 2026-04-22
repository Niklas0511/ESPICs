cmake_minimum_required(VERSION 4.2)
project(ESPICs)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_library(ESPICs
        src/ESPICs.cpp
        src/ESPICs.h
)