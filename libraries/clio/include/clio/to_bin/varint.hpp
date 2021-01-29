#pragma once
#include <clio/to_bin.hpp>
#include <clio/varint.hpp>

namespace clio {

template <typename S>
void to_bin(const varuint32& obj, S& stream) {
   varuint32_to_bin(obj.value, stream);
}

template <typename S>
void to_bin(const varint32& obj, S& stream) {
   varuint32_to_bin((uint32_t(obj.value) << 1) ^ uint32_t(obj.value >> 31), stream);
}

} /// namespace clio
