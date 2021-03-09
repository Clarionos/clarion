if(DEFINED IS_NATIVE)
    find_package(Threads REQUIRED)
    add_executable(clio_tests 
        clio_tests.cpp
        flat_views.cpp
        benchmark.cpp
        crypto.cpp
    )
    target_link_libraries(clio_tests clio clcrypto fc catch2-portable Threads::Threads)
endif()

if(DEFINED IS_WASM)
    add_executable(clio_tests 
        clio_tests.cpp
        flat_views.cpp
        benchmark.cpp
    )
    target_link_libraries(clio_tests clio catch2-portable boost)
endif()

set_target_properties(clio_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ROOT_BINARY_DIR})
