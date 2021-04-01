#include <eden.hpp>
#include <members.hpp>

namespace eden
{
   void eden::transfer_handler(eosio::name from,
                               eosio::name to,
                               const eosio::asset& quantity,
                               std::string memo)
   {
      print_f("transfer from name: %\n", from);

      eosio::check(to == get_self(), "only accepting transfers to us");
      eosio::check(quantity.symbol == default_token, "token must be a valid EOS");

      members{get_self()}.deposit(from, quantity);
   }
}  // namespace eden
