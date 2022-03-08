add_compile_options( -Wall -Wstrict-aliasing -fstrict-aliasing )

if(DEFINED IS_NATIVE)
    find_package(Threads REQUIRED)
    add_executable(clio-tests 
        fracpack.cpp
        clio_tests.cpp
#        flat_views.cpp
#        benchmark.cpp
#        crypto.cpp
    )
    target_link_libraries(clio-tests clio clcrypto fc catch2-portable Threads::Threads)
endif()

if(DEFINED IS_WASM)
    add_executable(clio-tests 
        fracpack.cpp
#        clio_tests.cpp
#        flat_views.cpp
#        benchmark.cpp
    )
    target_link_libraries(clio-tests clio catch2-portable boost)
endif()

set_target_properties(clio-tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ROOT_BINARY_DIR})

native_and_wasm_test(clio-tests)
