#include <clcrypto/base58.hpp>
#include <clio/murmur.hpp>
#include <cstring>
#include <stdexcept>

namespace clcrypto {

    namespace detail {
        // converts bytes to murmur64 checksumed base58
        std::string bytes_to_checksum_string( const char* data, uint8_t len ) {
            uint32_t hash = (uint32_t)clio::murmur64( data, len );

            char buffer[len+sizeof(hash)];
            memcpy( buffer, data, len );
            memcpy( buffer+len, &hash, sizeof(hash) );

            return to_base58( std::string_view(buffer, sizeof(buffer)) );
        }

        // converts murmur64 checksumed base58 to the orignal bytes
        // @throw std::runtime_error if the checksum doesn't match
        void checksumed_string_to_bytes( const std::string_view& str, char* data, uint8_t len ) {
            char buffer[len+sizeof(uint32_t)];
            from_base58( str, buffer, sizeof(buffer) );
            uint32_t hash = (uint32_t)clio::murmur64( buffer, len );
            if( memcmp( buffer+len, &hash, sizeof(hash) ) )
                throw std::runtime_error( "base58 checksum mismatch" );
            memcpy( data, buffer, len );
        }
    };

} // namespace clcrypto
