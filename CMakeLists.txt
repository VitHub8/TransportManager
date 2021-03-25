cmake_minimum_required(VERSION 3.16)
project(TRANSPORT)
find_package (Threads)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB TRANSPORT_SRC
        transport_manager.h
        transport_manager.cpp
        request.h
        request.cpp
        console_parser.h
        locator.h
        locator.cpp
        json.h
        json.cpp
        response.h
        response_console_out.cpp
        response_json_out.cpp
        main.cpp
        test_runner.h
        router.h
        graph.h
        )


add_executable(TRANSPORT ${TRANSPORT_SRC})

target_link_libraries(TRANSPORT ${CMAKE_THREAD_LIBS_INIT})
