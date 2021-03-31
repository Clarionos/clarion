#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>

namespace eden
{
   inline const eosio::symbol default_token{"EOS", 4};

   struct [[eosio::table("accounts"), eosio::contract("eden")]] account
   {
      eosio::name account;
      eosio::asset balance;

      uint64_t primary_key() const { return account.value; }
   };
   using accounts_tb = eosio::multi_index<"accounts"_n, account>;

   class [[eosio::contract("eden")]] accounts : public eosio::contract
   {
     public:
      using contract::contract;

      accounts(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds)
          : contract(receiver, code, ds), accounts_tb(receiver, receiver.value)
      {
      }

      [[eosio::on_notify("eosio.token::transfer")]] void transfer_handler(
          eosio::name from, eosio::name to, const eosio::asset& quantity, std::string memo);

      [[eosio::action]] void hi(eosio::name user);

     private:
      eden::accounts_tb accounts_tb;
   };

}  // namespace eden
