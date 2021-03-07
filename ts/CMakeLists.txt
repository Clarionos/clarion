project(ts)
cmake_minimum_required(VERSION 3.16.3)

configure_file(tsconfig.json tsconfig.json)

add_custom_target(dist ALL
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/node_modules
    COMMAND ln -sf ${CMAKE_BINARY_DIR}/node_modules .
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/package.json .
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/yarn.lock .
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/node_modules
    COMMAND yarn
    COMMAND npx tsc
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clwasmer
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clariond
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clintrinsics
    COMMAND cp -a compiled/libraries/clwasmer/src/* ${CMAKE_BINARY_DIR}/dist/clwasmer
    COMMAND cp -a compiled/programs/clariond/src/* ${CMAKE_BINARY_DIR}/dist/clariond
    COMMAND cp -a compiled/tests/clintrinsics/* ${CMAKE_BINARY_DIR}/dist/clintrinsics
)
