#pragma once
#include <clcrypto/base58.hpp>
#include <clio/reflect.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/elliptic_r1.hpp>

#include <variant>
#include <string>
#include <array>

namespace clcrypto {
    using std::variant;

    using fc::sha256;

    namespace detail {
        // converts bytes to murmur64 checksumed base58
        std::string bytes_to_checksumed_string( const char* data, uint8_t len );
        void checksumed_string_to_bytes( const std::string_view& str, char* data, uint8_t len );
        sha256 generate_r1();
        sha256 generate_k1();
    };

    enum ecc_curve {
        k1 = 0,
        r1 = 1
    };

    template<ecc_curve curve = k1>
    struct ecc_signature;

    template<ecc_curve curve = k1>
    struct ecc_public_key : public std::array<char,33> {
        using array<char,33>::array;

        ecc_public_key( const std::string_view& b58 ) {
            if constexpr ( curve == k1 ) {
                if( not b58.find( "PUBK1_" ) ) throw std::runtime_error( "public_key didn't start with SIGK1_" );
            }
            else if constexpr ( curve == k1 ) {
                if( not b58.find( "PUBR1_" ) ) throw std::runtime_error( "public_key didn't start with SIGR1_" );
            }
            detail::checksumed_string_to_bytes( std::string_view(b58.data()+6,b58.size()-6), data(), size() );
        }


        ecc_public_key( const ecc_signature<curve>& sig, const sha256& digest ) {
            if constexpr ( curve == k1 ) {
                fc::ecc::compact_signature cs;
                memcpy( cs.begin(), sig.data(), cs.size() );
                auto r = fc::ecc::public_key( cs, digest ).serialize();
                memcpy( data(), r.begin(), r.size() );
            }
            else {
                fc::crypto::r1::compact_signature cs;
                memcpy( cs.begin(), sig.data(), cs.size() );
                auto r = fc::crypto::r1::public_key( cs, digest ).serialize();
                memcpy( data(), r.begin(), r.size() );
            }
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


        ecc_private_key( const std::string_view& b58 ) {
            if constexpr ( curve == k1 ) {
                if( not b58.find( "PVTK1_" ) ) throw std::runtime_error( "private_key didn't start with SIGK1_" );
            }
            else if constexpr ( curve == k1 ) {
                if( not b58.find( "PVTR1_" ) ) throw std::runtime_error( "private_key didn't start with SIGR1_" );
            }
            detail::checksumed_string_to_bytes( std::string_view(b58.data()+6,b58.size()-6), data(), size() );
        }

        static auto generate() {
            ecc_private_key r;
            if constexpr ( curve == k1 ) {
                auto k = detail::generate_k1();
                static_assert( sizeof(k) == sizeof(r) );
                memcpy( &r, &k, sizeof(k) );
            } else {
                auto k = detail::generate_r1();
                static_assert( sizeof(k) == sizeof(r) );
                memcpy( &r, &k, sizeof(k) );
            }
            return r;
        }

        ecc_signature<curve> sign( const sha256& digest )const;

        ecc_public_key<curve> get_public_key()const {
            if constexpr ( curve == k1 ) {
                sha256 d;
                memcpy( &d, this, sizeof(d) );
                auto pri = fc::ecc::private_key::regenerate( d );
                auto r = pri.get_public_key().serialize();
                ecc_public_key<curve> pub;
                memcpy( pub.data(), r.begin(), r.size() );
                return pub;
            }
            return {};
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

    template<ecc_curve curve>
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

        ecc_public_key<curve> recover( const sha256& digest )const {
            return ecc_public_key<curve>( *this, digest );
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


    template<ecc_curve Curve>
    ecc_signature<Curve> ecc_private_key<Curve>::sign( const sha256& digest )const {
        ecc_signature<Curve> sig;
        sha256 sec;
        std::cerr<<"size: " << this->size() <<"\n";
        memcpy( sec.data(), data(), this->size() );
        if constexpr ( Curve == k1 ) {
            auto pk = fc::ecc::private_key::regenerate( sec );
            auto s = pk.sign_compact( digest );
            memcpy( sig.data(), s.begin(), s.size() );
        } else if constexpr ( Curve == r1 ) {
            auto pk = fc::crypto::r1::private_key::regenerate( sec );
            auto s = pk.sign_compact( digest );
            memcpy( sig.data(), s.begin(), s.size() );
        }
        return sig;
    }


    using public_key  = variant< ecc_public_key<k1>, ecc_public_key<r1> >;
    using private_key = variant< ecc_private_key<k1>, ecc_private_key<r1> >;
    using signature   = variant< ecc_signature<k1>, ecc_signature<r1> >;

    auto get_public_key( const private_key& p ) {
        public_key pub;
        std::visit( [&]( const auto& k ){ return pub = k.get_public_key(); }, p );
        return pub;
    }
    auto sign( const private_key& p, const sha256& digest ) {
        signature sig;
        std::visit( [&]( const auto& k ){ return sig = k.sign(digest); }, p );
        return sig;
    }

    using k1pub = ecc_public_key<k1>;
    using r1pub = ecc_public_key<r1>;
    using k1pri = ecc_private_key<k1>;
    using r1pri = ecc_private_key<r1>;
    using k1sig = ecc_signature<k1>;
    using r1sig = ecc_signature<r1>;

    static_assert( std::is_trivially_copyable_v<k1pub> );
    static_assert( std::is_trivially_copyable_v<r1pub> );
    static_assert( std::is_trivially_copyable_v<k1pri> );
    static_assert( std::is_trivially_copyable_v<r1pri> );
    static_assert( std::is_trivially_copyable_v<k1sig> );
    static_assert( std::is_trivially_copyable_v<r1sig> );

    CLIO_REFLECT_TYPENAME( k1pub );
    CLIO_REFLECT_TYPENAME( r1pub );
    CLIO_REFLECT_TYPENAME( k1pri );
    CLIO_REFLECT_TYPENAME( r1pri );
    CLIO_REFLECT_TYPENAME( k1sig );
    CLIO_REFLECT_TYPENAME( r1sig );

};

