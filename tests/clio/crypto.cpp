#include <catch2/catch.hpp>
#include <boost/core/demangle.hpp>
#include <iostream>

#include <clio/to_json/varint.hpp>
#include <clio/from_json/varint.hpp>
#include <clio/from_bin/varint.hpp>
#include <clio/to_bin/varint.hpp>
#include <clio/json/any.hpp>
#include <clio/compress.hpp>

#include <clio/translator.hpp>
#include <clio/to_json/map.hpp>
#include <clio/bytes/to_json.hpp>
#include <clio/bytes/from_json.hpp>
#include <fstream>

#include <clio/flatbuf.hpp>
#include <chrono>

#include <clcrypto/keys.hpp>
#include <fc/crypto/public_key.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/crypto/sha256.hpp>

struct identity {
    clcrypto::private_key   priv;
    clcrypto::public_key    pub; 
    clcrypto::signature     sig;
    std::string             hello;
    clcrypto::sha256        digest;
};

CLIO_REFLECT( identity, hello, digest, priv, pub, sig ) 

TEST_CASE( "keygen" ) {
    std::cout << "KEYGEN\n";
    auto Priv  = fc::crypto::private_key::generate_r1();
    auto Pub   = Priv.get_public_key();

    std::cout << Priv.to_string() << "\n";
    std::cout << Pub.to_string() << "\n";

    clcrypto::k1pri privk = clcrypto::k1pri::generate();
    clcrypto::k1pub pubk  = privk.get_public_key(); //clcrypto::k1pub();

    //auto hashhello = fc::sha256::hash( "hello world", 11 );
    auto hashhello = fc::sha256::hash( std::string("hello world") );
    auto sig = privk.sign(hashhello);

    std::cout << std::string(pubk) << "\n";
    std::cout << std::string(privk) << "\n";
    std::cout << hashhello.str() <<"\n";
    std::cout << std::string(sig) <<"\n";

    auto rek = sig.recover( hashhello );
    std::cout << std::string( rek ) <<"\n";

    identity id; 
    id.priv = clcrypto::r1pri::generate();
    id.pub  = clcrypto::get_public_key( id.priv );
    id.sig  = clcrypto::sign( id.priv, id.digest );
    std::cout << clio::to_json(id) <<"\n";
}
