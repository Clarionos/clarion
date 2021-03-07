if(DEFINED IS_NATIVE)
    add_subdirectory(clio)
endif()

if(DEFINED IS_WASM)
    add_subdirectory(clintrinsics)
    add_subdirectory(web)
endif()
