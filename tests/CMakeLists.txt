add_library(catch2-portable INTERFACE)
target_link_libraries(catch2-portable INTERFACE Catch2)

if(DEFINED IS_WASM)
    target_link_libraries(catch2-portable INTERFACE clintrinsics)
    target_compile_options(catch2-portable INTERFACE
        -fno-exceptions
        -DCATCH_CONFIG_DISABLE_EXCEPTIONS
        -DCATCH_CONFIG_NO_POSIX_SIGNALS
    )
endif()

add_subdirectory(clio)

if(DEFINED IS_WASM)
    add_subdirectory(clintrinsics)
    add_subdirectory(web)
endif()
