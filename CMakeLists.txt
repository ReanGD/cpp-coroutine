cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PROJECT coroutine)
project(${PROJECT} CXX)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(src/)
include_directories(include/)

find_package(Boost COMPONENTS thread system context REQUIRED)
set(LIBS ${Boost_LIBRARIES})

aux_source_directory(src/ SRC_MAIN)
set(SRCS ${SRC_MAIN})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fcxx-exceptions -fexceptions -Weverything -pedantic -Wno-c++98-compat -Wno-exit-time-destructors -Wno-global-constructors -Wno-padded")

add_library(${PROJECT} STATIC ${SRCS})
target_link_libraries(${PROJECT} ${LIBS})
set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME ${PROJECT}_static)
