if(DEFINED IS_NATIVE)
    add_subdirectory(fc)
    add_subdirectory(net)
    add_subdirectory(appbase)
    add_subdirectory(clcrypto)
endif()

add_subdirectory(clio)
