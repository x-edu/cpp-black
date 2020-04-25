cmake_minimum_required(VERSION 3.16)
project(phone_book)

set(CMAKE_CXX_STANDARD 17)

find_package(
        Protobuf REQUIRED
)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
protobuf_generate_cpp(
        PROTO_SRCS
        PROTO_HDRS
        contact.proto
)

add_executable(
        main
        main.cpp
        iterator_range.h
        phone_book.h
        test_runner.h
        ${PROTO_SRCS}
        ${PROTO_HDRS}
        phone_book.cpp)

target_link_libraries(main ${Protobuf_LIBRARIES})
