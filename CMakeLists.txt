project(clarion)
cmake_minimum_required(VERSION 3.16.3)
include(ExternalProject)

if(NOT DEFINED WASI_SDK_PREFIX AND DEFINED ENV{WASI_SDK_PREFIX})
    set(WASI_SDK_PREFIX $ENV{WASI_SDK_PREFIX})
endif()

if(NOT DEFINED SKIP_NATIVE OR NOT SKIP_NATIVE)
    ExternalProject_Add(native
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/native
        BINARY_DIR native
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )
endif()

if(DEFINED WASI_SDK_PREFIX)
    ExternalProject_Add(wasm
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasm
        BINARY_DIR wasm
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/wasm/toolchain.cmake
            -DWASI_SDK_PREFIX=${WASI_SDK_PREFIX}
            -DCMAKE_SYSROOT=${WASI_SDK_PREFIX}/share/wasi-sysroot
    )
else()
    message(WARNING "WASI_SDK_PREFIX isn't defined; skipping wasm")
endif()

if(NOT DEFINED SKIP_TS OR NOT SKIP_TS)
    add_subdirectory(ts)
endif()
