#pragma once

#include <clio/from_json.hpp>
#include <clio/varint.hpp>

namespace clio {

template <typename S>
void from_json(varuint32& obj, S& stream) {
   from_json(obj.value, stream);
}

template <typename S>
void from_json(varint32& obj, S& stream) {
   from_json(obj.value, stream);
}

} /// namespace clio
