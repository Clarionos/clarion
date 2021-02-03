#pragma once
#include <fc/crypto/sha256.hpp>

namespace clcrypto {
    using sha256 = fc::sha256;

    CLIO_REFLECT_TYPENAME( sha256 );

} /// clcrypto

