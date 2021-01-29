#pragma once
#include <clio/bytes.hpp>
#include <clio/from_json.hpp>

namespace clio {

template <typename S>
void from_json(bytes& obj, S& stream) {
   clio::from_json_hex(obj.data, stream);
}

} // namespace clio
