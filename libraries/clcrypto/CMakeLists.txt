add_library( clcrypto 
             keys.cpp 
             base58.cpp 
             )


target_include_directories( clcrypto PUBLIC include  )
target_link_libraries( clcrypto clio fc )


# some crypto operations require openssl which isn't available in wasm
add_library( clcrypto_ssl 
             keys.cpp
             openssl.cpp 
             )
target_include_directories( clcrypto_ssl PUBLIC include  )
target_link_libraries( clcrypto_ssl fc clio OpenSSL::SSL)
