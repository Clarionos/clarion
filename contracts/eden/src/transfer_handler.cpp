#include <accounts.hpp>

namespace eden
{
   void accounts::transfer_handler(eosio::name from,
                                   eosio::name to,
                                   const eosio::asset& quantity,
                                   std::string memo)
   {
      print_f("transfer from Name: %\n", from);

      eosio::check(to == get_self(), "only accepting transfers to us");
      eosio::check(quantity.symbol == default_token, "only EOS");
      eosio::check(quantity.amount > 0, "insufficient value");

      auto itr = accounts_tb.find(from.value);
      if (itr == accounts_tb.end())
      {
         accounts_tb.emplace(get_self(), [&](auto& row) {
            row.account = from;
            row.balance = quantity;
         });
      }
      else
      {
         accounts_tb.modify(itr, eosio::same_payer, [&](auto& row) { row.balance += quantity; });
      }
   }
}  // namespace eden
