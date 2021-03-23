cmake_minimum_required(VERSION 3.16.3)
project(clarion)
include(ExternalProject)

if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported; create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt and ${CMAKE_SOURCE_DIR}/CMakeFiles")
endif()

if(NOT DEFINED WASI_SDK_PREFIX AND DEFINED ENV{WASI_SDK_PREFIX})
    set(WASI_SDK_PREFIX $ENV{WASI_SDK_PREFIX})
endif()

file(WRITE ${CMAKE_BINARY_DIR}/CTestTestfile.cmake)

if(NOT DEFINED SKIP_NATIVE OR NOT SKIP_NATIVE)
    ExternalProject_Add(native
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/native
        BINARY_DIR native
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
        TEST_EXCLUDE_FROM_MAIN 1
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )
    file(APPEND ${CMAKE_BINARY_DIR}/CTestTestfile.cmake "subdirs(\"native\")\n")
    ExternalProject_Add_StepTargets(native test)
endif()

if(DEFINED WASI_SDK_PREFIX)
    ExternalProject_Add(wasm
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wasm
        BINARY_DIR wasm
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
        TEST_EXCLUDE_FROM_MAIN 1
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/wasm/toolchain.cmake
            -DWASI_SDK_PREFIX=${WASI_SDK_PREFIX}
            -DCMAKE_SYSROOT=${WASI_SDK_PREFIX}/share/wasi-sysroot
    )
    file(APPEND ${CMAKE_BINARY_DIR}/CTestTestfile.cmake "subdirs(\"wasm\")\n")
    ExternalProject_Add_StepTargets(wasm test)
else()
    message(WARNING "WASI_SDK_PREFIX isn't defined; skipping wasm")
endif()

########## TS PACKAGES ####################
if(NOT DEFINED SKIP_TS OR NOT SKIP_TS)
    add_custom_target(yarn ALL
        DEPENDS wasm
        COMMAND yarn
        COMMAND CLARION_WASM_PATH=${CMAKE_BINARY_DIR}/a.wasm yarn build
    )

    add_custom_target(packages ALL
        DEPENDS yarn
        COMMAND rm -rf ${CMAKE_BINARY_DIR}/dist

        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/tester
        COMMAND cp -a ${CMAKE_CURRENT_SOURCE_DIR}/packages/tester/dist/* ${CMAKE_BINARY_DIR}/dist/tester

        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clariond
        COMMAND cp -a ${CMAKE_CURRENT_SOURCE_DIR}/packages/d/dist/* ${CMAKE_BINARY_DIR}/dist/clariond
        COMMAND mkdir -p clariondata

        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/pwa
        COMMAND cp -a ${CMAKE_CURRENT_SOURCE_DIR}/packages/pwa/dist/* ${CMAKE_BINARY_DIR}/dist/pwa

        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/app
        COMMAND cp -a ${CMAKE_CURRENT_SOURCE_DIR}/packages/app/dist/* ${CMAKE_BINARY_DIR}/dist/app
    )

    add_custom_target(package-wasm
        DEPENDS wasm
        COMMAND cp a.wasm ${CMAKE_CURRENT_SOURCE_DIR}/packages/d/dist/clarion.wasm
    )

    add_custom_target(dev
        DEPENDS yarn package-wasm
        COMMAND CLARION_WASM_PATH=${CMAKE_BINARY_DIR}/a.wasm yarn dev --stream
    )
endif()
