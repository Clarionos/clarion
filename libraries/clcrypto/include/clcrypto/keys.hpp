#pragma once
#include <clcrypto/base58.hpp>
#include <clio/reflect.hpp>

#include <variant>
#include <string>
#include <array>

namespace clcrypto {
    using std::variant;

    namespace detail {
        // converts bytes to murmur64 checksumed base58
        std::string bytes_to_checksumed_string( const char* data, uint8_t len );
        void checksumed_string_to_bytes( const std::string_view& str, char* data, uint8_t len );
    };

    enum ecc_curve {
        k1 = 0,
        r1 = 1
    };

    template<ecc_curve curve = k1>
    struct ecc_public_key : public std::array<char,33> {
        using array<char,33>::array;
        static_assert( std::is_trivially_copyable_v<ecc_public_key> );

        ecc_public_key( const std::string_view& b58 ) {
            if constexpr ( curve == k1 ) {
                if( not b58.find( "PUBK1_" ) ) throw std::runtime_error( "public_key didn't start with SIGK1_" );
            }
            else if constexpr ( curve == k1 ) {
                if( not b58.find( "PUBR1_" ) ) throw std::runtime_error( "public_key didn't start with SIGR1_" );
            }
            detail::checksumed_string_to_bytes( std::string_view(b58.data()+6,b58.size()-6), data(), size() );
        }

        operator std::string()const {
            // intentionally using PUBK1_ instead of PUB_K1_ to make different
            // from eosio because the checksum is murmur not ripemd 
            if constexpr ( curve == k1 )
                return "PUBK1_" + detail::bytes_to_checksumed_string( data(), size() );
            else if constexpr ( curve == r1 )
                return "PUBR1_" + detail::bytes_to_checksumed_string( data(), size() );
            else {
                static_assert( curve == k1 or curve == r1, "unknown curve type" );
            }
        }
    };

    template<ecc_curve curve = k1>
    struct ecc_private_key : public std::array<char,32> {
        using array<char,32>::array;
        static_assert( std::is_trivially_copyable_v<ecc_private_key> );


        ecc_private_key( const std::string_view& b58 ) {
            if constexpr ( curve == k1 ) {
                if( not b58.find( "PVTK1_" ) ) throw std::runtime_error( "private_key didn't start with SIGK1_" );
            }
            else if constexpr ( curve == k1 ) {
                if( not b58.find( "PVTR1_" ) ) throw std::runtime_error( "private_key didn't start with SIGR1_" );
            }
            detail::checksumed_string_to_bytes( std::string_view(b58.data()+6,b58.size()-6), data(), size() );
        }

        operator std::string()const {
            // intentionally using PRVK1_ instead of PRV_K1_ to make different
            // from eosio because the checksum is murmur not ripemd 
            if constexpr ( curve == k1 )
                return "PVTK1_" + detail::bytes_to_checksumed_string( data(), size() );
            else if constexpr ( curve == r1 )
                return "PVTR1_" + detail::bytes_to_checksumed_string( data(), size() );
            else {
                static_assert( curve == k1 or curve == r1, "unknown curve type" );
            }
        }
    };

    template<ecc_curve curve = k1>
    struct ecc_signature : public std::array<char,65> {
        using array<char,65>::array;

        ecc_signature( const std::string_view& b58 ) {
            if constexpr ( curve == k1 ) {
                if( not b58.find( "SIGK1_" ) ) throw std::runtime_error( "signature didn't start with SIGK1_" );
            }
            else if constexpr ( curve == k1 ) {
                if( not b58.find( "SIGR1_" ) ) throw std::runtime_error( "signature didn't start with SIGR1_" );
            }
            detail::checksumed_string_to_bytes( std::string_view(b58.data()+6,b58.size()-6), data(), size() );
        }

        operator std::string()const {
            if constexpr ( curve == k1 )
                return "SIGK1_" + detail::bytes_to_checksumed_string( data(), size() );
            else if constexpr ( curve == r1 )
                return "SIGR1_" + detail::bytes_to_checksumed_string( data(), size() );
            else {
                static_assert( curve == k1 or curve == r1, "unknown curve type" );
            }
        }
    };
    static_assert( std::is_trivially_copyable_v<ecc_signature<>> );

    using public_key  = variant< ecc_public_key<k1>, ecc_public_key<r1> >;
    using private_key = variant< ecc_private_key<k1>, ecc_private_key<r1> >;
    using signature   = variant< ecc_signature<k1>, ecc_signature<r1> >;

    using k1pub = ecc_public_key<k1>;
    using r1pub = ecc_public_key<r1>;
    using k1pri = ecc_private_key<k1>;
    using r1pri = ecc_private_key<r1>;
    using k1sig = ecc_signature<k1>;
    using r1sig = ecc_signature<r1>;

    B1IO_REFLECT_TYPENAME( k1pub );
    B1IO_REFLECT_TYPENAME( r1pub );
    B1IO_REFLECT_TYPENAME( k1pri );
    B1IO_REFLECT_TYPENAME( r1pri );
    B1IO_REFLECT_TYPENAME( k1sig );
    B1IO_REFLECT_TYPENAME( r1sig );
    
    /* This should be covered by trivially_copyable impl of serialization functions
     *
    template<ecc_curve curve, typename Stream>
    void to_bin( const ecc_public_key<curve>& pub, Stream& s ) {
        s.write( pub.data(), pub.size() );
    }

    template<ecc_curve curve, typename Stream>
    void from_bin( ecc_public_key<curve>& pub, Stream& s ) {
        s.read( pub.data(), pub.size() );
    }

    template<ecc_curve curve, typename Stream>
    void to_bin( const ecc_private_key<curve>& pub, Stream& s ) {
        s.write( pub.data(), pub.size() );
    }

    template<ecc_curve curve, typename Stream>
    void from_bin( ecc_private_key<curve>& pub, Stream& s ) {
        s.read( pub.data(), pub.size() );
    }

    template<ecc_curve curve, typename Stream>
    void to_bin( const ecc_signature<curve>& pub, Stream& s ) {
        s.write( pub.data(), pub.size() );
    }

    template<ecc_curve curve, typename Stream>
    void from_bin( ecc_signature<curve>& pub, Stream& s ) {
        s.read( pub.data(), pub.size() );
    }
    */

};
