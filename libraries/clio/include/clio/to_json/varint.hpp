#pragma once
#include <clio/to_json.hpp>
#include <clio/varint.hpp>

namespace clio {

template <typename S>
void to_json(const varuint32& obj, S& stream) {
   to_json(obj.value, stream);
}

template <typename S>
void to_json(const varint32& obj, S& stream) {
   to_json(obj.value, stream);
}

} // namespace clio
