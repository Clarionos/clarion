project(ts)
cmake_minimum_required(VERSION 3.16.3)

configure_file(tsconfig.json tsconfig.json)
configure_file(pwa.tsconfig.json pwa.tsconfig.json)
configure_file(pwa.webpack.config.js pwa.webpack.config.js)

add_custom_target(yarn
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/node_modules
    COMMAND ln -sf ${CMAKE_BINARY_DIR}/node_modules .
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/package.json .
    COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/yarn.lock .
    COMMAND ln -sf ${CMAKE_SOURCE_DIR} src
    COMMAND yarn
)

add_custom_target(dist ALL
    DEPENDS yarn
    COMMAND npx webpack --config pwa.webpack.config.js
    COMMAND npx tsc
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clwasmer
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clariond
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist/clintrinsics
    COMMAND cp -a compiled/libraries/clwasmer/src/* ${CMAKE_BINARY_DIR}/dist/clwasmer
    COMMAND cp -a compiled/programs/clariond/src/* ${CMAKE_BINARY_DIR}/dist/clariond
    COMMAND cp -a compiled/tests/clintrinsics/* ${CMAKE_BINARY_DIR}/dist/clintrinsics
)

add_custom_target(pwa ALL
    DEPENDS yarn wasm
    COMMAND npx webpack --config pwa.webpack.config.js
)

add_custom_target(pwa-dev
    DEPENDS yarn wasm
    COMMAND npx webpack serve --config pwa.webpack.config.js --open
)
