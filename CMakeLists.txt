cmake_minimum_required(VERSION 2.8)
project(rw)

include_directories(include)

set(SOURCE_FILES
        src/Cluster.cpp
        src/main.cpp
        src/Node.cpp
        )

add_compile_options(-std=c++11)
SET(CMAKE_CXX_COMPILER g++)
SET(CMAKE_CXX_FLAGS -O3)

add_executable(rw ${SOURCE_FILES})
