cmake_minimum_required(VERSION 3.16)
project(Transport_manager)
find_package (Threads)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-O2")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB Transport_manager_SRC
        descriptions.cpp
        descriptions.h
        graph.h
        json.cpp
        json.h
        main.cpp
        profile.h
        requests.cpp
        requests.h
        router.h
        sphere.cpp
        sphere.h
        test_runner.h
        transport_catalog.cpp
        transport_catalog.h
        transport_router.cpp
        transport_router.h
        utils.cpp
        utils.h
        svg.cpp
        svg.h
        render.h
        render.cpp
        )

add_executable(Transport_manager ${Transport_manager_SRC})

target_link_libraries(Transport_manager ${CMAKE_THREAD_LIBS_INIT})
