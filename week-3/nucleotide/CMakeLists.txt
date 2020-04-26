cmake_minimum_required(VERSION 3.16)
project(nucleotide)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/../../lib)

add_executable(
        nucleotide
        main.cpp
        nucleotide.h
)
