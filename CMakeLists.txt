cmake_minimum_required(VERSION 3.6)
project(async)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(async ${SOURCE_FILES})

TARGET_LINK_LIBRARIES( async pthread)