#include <members.hpp>

namespace eden
{
   void members::deposit(eosio::name member, eosio::asset quantity)
   {
      eosio::check(quantity >= minimum_membership_donation,
                   "minimum donation value is 10.0000 EOS");

      auto itr = members_tb.find(member.value);
      if (itr == members_tb.end())
      {
         members_tb.emplace(contract, [&](auto& row) {
            row.member = member;
            row.balance = quantity;
            row.status = member_status::pending;
         });
      }
      else
      {
         members_tb.modify(itr, eosio::same_payer, [&](auto& row) { row.balance += quantity; });
      }
   }
}  // namespace eden
