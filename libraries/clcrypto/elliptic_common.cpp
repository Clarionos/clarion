#include <clcrypto/openssl.hpp>
//#include <fc/crypto/ripemd160.hpp>

/*
#ifdef defined(__FreeBSD__)
# include <stdlib.h>
#else
# include <alloca.h>
#endif
*/

/* stuff common to all ecc implementations */

//#define BTC_EXT_PUB_MAGIC   (0x0488B21E)
//#define BTC_EXT_PRIV_MAGIC  (0x0488ADE4)

namespace clcrypto {



    /*
    namespace detail {
        typedef fc::array<char,37> chr37;

        fc::sha256 _left( const fc::sha512& v )
        {
            fc::sha256 result;
            memcpy( result.data(), v.data(), 32 );
            return result;
        }

        fc::sha256 _right( const fc::sha512& v )
        {
            fc::sha256 result;
            memcpy( result.data(), v.data() + 32, 32 );
            return result;
        }

        static void _put( unsigned char** dest, unsigned int i)
        {
            *(*dest)++ = (i >> 24) & 0xff;
            *(*dest)++ = (i >> 16) & 0xff;
            *(*dest)++ = (i >>  8) & 0xff;
            *(*dest)++ =  i        & 0xff;
        }


        static chr37 _derive_message( char first, const char* key32, int i )
        {
            chr37 result;
            unsigned char* dest = (unsigned char*) result.begin();
            *dest++ = first;
            memcpy( dest, key32, 32 ); dest += 32;
            _put( &dest, i );
            return result;
        }

        chr37 _derive_message( const public_key_data& key, int i )
        {
            return _derive_message( *key.begin(), key.begin() + 1, i );
        }


        const ec_group& get_curve()
        {
            static const ec_group secp256k1( EC_GROUP_new_by_curve_name( NID_secp256k1 ) );
            return secp256k1;
        }

        static private_key_secret _get_curve_order()
        {
            const ec_group& group = get_curve();
            bn_ctx ctx(BN_CTX_new());
            ssl_bignum order;
            FC_ASSERT( EC_GROUP_get_order( group, order, ctx ) );
            private_key_secret bin;
            FC_ASSERT( BN_num_bytes( order ) == static_cast<int>(bin.data_size()) );
            FC_ASSERT( BN_bn2bin( order, (unsigned char*) bin.data() ) == static_cast<int>(bin.data_size()) );
            return bin;
        }

        const private_key_secret& get_curve_order()
        {
            static private_key_secret order = _get_curve_order();
            return order;
        }

        static private_key_secret _get_half_curve_order()
        {
            const ec_group& group = get_curve();
            bn_ctx ctx(BN_CTX_new());
            ssl_bignum order;
            FC_ASSERT( EC_GROUP_get_order( group, order, ctx ) );
            BN_rshift1( order, order );
            private_key_secret bin;
            FC_ASSERT( BN_num_bytes( order ) == static_cast<int>(bin.data_size()) );
            FC_ASSERT( BN_bn2bin( order, (unsigned char*) bin.data() ) == static_cast<int>(bin.data_size()) );
            return bin;
        }

        const private_key_secret& get_half_curve_order()
        {
            static private_key_secret half_order = _get_half_curve_order();
            return half_order;
        }
    }
    */

    public_key public_key::from_key_data( const public_key_data &data ) {
        return public_key(data);
    }


    /*
    unsigned int public_key::fingerprint() const
    {
        public_key_data key = serialize();
        ripemd160 hash = ripemd160::hash( sha256::hash( key.begin(), key.size() ) );
        unsigned char* fp = (unsigned char*) hash._hash;
        return (fp[0] << 24) | (fp[1] << 16) | (fp[2] << 8) | fp[3];
    }
    */

    bool public_key::is_canonical( const compact_signature& c ) {
        return !(c.data[1] & 0x80)
               && !(c.data[1] == 0 && !(c.data[2] & 0x80))
               && !(c.data[33] & 0x80)
               && !(c.data[33] == 0 && !(c.data[34] & 0x80));
    }

    private_key private_key::generate_from_seed( const fc::sha256& seed, const fc::sha256& offset )
    {
        ssl_bignum z;
        BN_bin2bn((unsigned char*)&offset, sizeof(offset), z);

        ec_group group(EC_GROUP_new_by_curve_name(NID_secp256k1));
        bn_ctx ctx(BN_CTX_new());
        ssl_bignum order;
        EC_GROUP_get_order(group, order, ctx);

        // secexp = (seed + z) % order
        ssl_bignum secexp;
        BN_bin2bn((unsigned char*)&seed, sizeof(seed), secexp);
        BN_add(secexp, secexp, z);
        BN_mod(secexp, secexp, order, ctx);

        fc::sha256 secret;
        FC_ASSERT(BN_num_bytes(secexp) <= int64_t(sizeof(secret)));
        auto shift = sizeof(secret) - BN_num_bytes(secexp);
        BN_bn2bin(secexp, ((unsigned char*)&secret)+shift);
        return regenerate( secret );
    }

    fc::sha256 private_key::get_secret( const EC_KEY * const k )
    {
       if( !k )
       {
          return fc::sha256();
       }

       fc::sha256 sec;
       const BIGNUM* bn = EC_KEY_get0_private_key(k);
       if( bn == NULL )
       {
         FC_THROW_EXCEPTION( exception, "get private key failed" );
       }
       int nbytes = BN_num_bytes(bn);
       BN_bn2bin(bn, &((unsigned char*)&sec)[32-nbytes] );
       return sec;
    }

    private_key private_key::generate()
    {
       EC_KEY* k = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !k ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );
       if( !EC_KEY_generate_key( k ) )
       {
          FC_THROW_EXCEPTION( exception, "ecc key generation error" );

       }

       return private_key( k );
    }


} // clcrypto

