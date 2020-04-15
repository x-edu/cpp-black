cmake_minimum_required(VERSION 3.15)

project(transport_catalog_part_f)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        transport_catalog_part_f
        main.cpp
        test_runner.h
        tests.h
        tests.cpp
        svg.cpp
        svg.h
        # original.cpp
)
