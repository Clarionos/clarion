#pragma once
#include <clio/reflect.hpp>
#include <vector>

namespace clio {

struct bytes {
   std::vector<char> data;
};

B1IO_REFLECT(bytes, data);
//EOSIO_COMPARE(bytes);

} // namespace clio 
