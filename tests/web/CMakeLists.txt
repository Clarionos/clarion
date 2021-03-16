add_executable(a a.cpp)
target_link_libraries(a clintrinsics)
target_compile_options(a PUBLIC -fno-exceptions)
set_target_properties(a PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ROOT_BINARY_DIR})
