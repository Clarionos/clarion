project(wasm)
cmake_minimum_required(VERSION 3.16.3)

set(IS_WASM YES)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(../libraries libraries)
add_subdirectory(../programs programs)
add_subdirectory(../tests tests)
add_subdirectory(../external external)
add_subdirectory(boost)
