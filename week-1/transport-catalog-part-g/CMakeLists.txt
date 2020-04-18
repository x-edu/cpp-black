cmake_minimum_required(VERSION 3.15)
project(transport_catalog)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(TESTS=1)

add_executable(
        transport_catalog

        main.cpp
        utils.cpp
        utils.h
        json.cpp
        json.h
        sphere.cpp
        sphere.h
        graph.cpp
        graph.h
        router.cpp
        router.h
        descriptions.cpp
        descriptions.h
        requests.cpp
        requests.h
        transport_router.cpp
        transport_router.h
        transport_catalog.cpp
        transport_catalog.h
        svg.cpp
        svg.h

        test_runner.h
        tests.cpp
        tests.h
        renderer.cpp renderer.h)
