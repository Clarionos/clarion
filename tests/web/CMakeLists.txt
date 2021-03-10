add_executable(a a.cpp)
target_link_libraries(a clintrinsics)
set_target_properties(a PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ROOT_BINARY_DIR})
