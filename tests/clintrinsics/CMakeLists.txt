add_executable(test-clintrinsics a.cpp)
target_link_libraries(test-clintrinsics clintrinsics catch2-portable)
set_target_properties(test-clintrinsics PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ROOT_BINARY_DIR})

wasm_test(test-clintrinsics)
