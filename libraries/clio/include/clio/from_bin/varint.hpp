#pragma once

#include <clio/from_bin.hpp>
#include <clio/varint.hpp>

namespace clio {

template <typename S>
void from_bin(varuint32& obj, S& stream) {
   varuint32_from_bin(obj.value, stream);
}

template <typename S>
void from_bin(varint32& obj, S& stream) {
   varint32_from_bin(obj.value, stream);
}

} /// namespace clio
