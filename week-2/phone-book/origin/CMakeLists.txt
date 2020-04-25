cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_path(TEST_RUNNER_PATH test_runner.h)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${TEST_RUNNER_PATH})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS contact.proto)

add_executable(addressbook ${PROTO_SRCS} ${PROTO_HDRS} main.cpp address_book.cpp)
target_link_libraries(addressbook ${Protobuf_LIBRARIES})