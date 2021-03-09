cmake_minimum_required(VERSION 3.16.3)
cmake_policy(VERSION 3.16)
project(wasm)
enable_testing()

set(IS_WASM YES)
set(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
set(ROOT_BINARY_DIR ${CMAKE_BINARY_DIR}/..)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(native_test N)
endfunction()

function(wasm_test N)
    add_test(
        NAME w-${N}
        WORKING_DIRECTORY ${ROOT_BINARY_DIR}
        COMMAND node dist/tester ${N}.wasm
    )
    set_tests_properties(w-${N} PROPERTIES ENVIRONMENT NODE_PATH=dist)
endfunction()

function(native_and_wasm_test N)
    wasm_test(${N})
endfunction()

add_subdirectory(../libraries libraries)
add_subdirectory(../programs programs)
add_subdirectory(../tests tests)
add_subdirectory(../external external)
add_subdirectory(boost)
