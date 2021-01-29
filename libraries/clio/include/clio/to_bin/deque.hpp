#pragma once
#include <clio/to_bin.hpp>
#include <deque>

namespace clio {

    template <typename T, typename S>
    result<void> to_bin(const std::deque<T>& obj, S& stream) {
       return to_bin_range(obj, stream);
    }

} // clio 
