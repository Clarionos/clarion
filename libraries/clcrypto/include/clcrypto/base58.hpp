#pragma once
#include <string>
#include <string_view>

namespace clcrypto {

 std::string to_base58( const std::string_view& data );
 void from_base58( const std::string_view& b58, char* data, uint32_t data_size );

} /// namespace clcrypto
