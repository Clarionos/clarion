#add_subdirectory( quill )
add_subdirectory( Catch2 )
add_subdirectory( secp256k1 )

add_library( rapidjson INTERFACE )
target_include_directories( rapidjson INTERFACE rapidjson/include )
