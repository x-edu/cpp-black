cmake_minimum_required(VERSION 3.16)
project(mython_lexer)

set(CMAKE_CXX_STANDARD 17)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
        mython_lexer
        lexer_test.cpp
        lexer.cpp
)