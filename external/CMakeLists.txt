if(DEFINED IS_NATIVE)
    add_subdirectory(Catch2)
    add_subdirectory(secp256k1)
endif()

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson/include)
