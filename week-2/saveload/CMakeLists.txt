cmake_minimum_required(VERSION 3.16)
project(saveload)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        saveload
        main.cpp
        saveload.h
        test_runner.h
)