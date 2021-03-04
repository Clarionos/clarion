project(clarion)
cmake_minimum_required(VERSION 3.16.3)
include(ExternalProject)

ExternalProject_Add(native
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/native
    BINARY_DIR native
    INSTALL_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)
