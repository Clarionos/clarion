#include <accounts/accounts.hpp>

namespace eden {

void accounts::hi(eosio::name user) { eosio::print("Hello, ", user); }

} // namespace eden
