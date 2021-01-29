#pragma once
#include <clio/bytes.hpp>
#include <clio/to_json.hpp>

namespace clio {

template <typename S>
void to_json(const bytes& obj, S& stream) {
   clio::to_json_hex(obj.data.data(), obj.data.size(), stream);
}

} // namespace clio
