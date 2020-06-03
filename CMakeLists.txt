cmake_minimum_required(VERSION 3.16)
project(fix_parser)

set(CMAKE_CXX_STANDARD 17)

INCLUDE_DIRECTORIES(src)

file(GLOB SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(fix_parser main.cpp ${SOURCES})  