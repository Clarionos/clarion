add_executable(test-clintrinsics.wasm a.cpp)
target_link_libraries(test-clintrinsics.wasm clintrinsics Catch2)
target_compile_options(test-clintrinsics.wasm PUBLIC -fno-exceptions)
