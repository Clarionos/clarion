#pragma once

#include <constants.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>

namespace eden
{
   inline const eosio::asset minimum_membership_donation{10, default_token};

   using member_status_type = uint8_t;
   enum member_status : member_status_type
   {
      pending = 0,
      active = 1,
      expired = 2
   };

   struct [[eosio::table("members"), eosio::contract("eden")]] member
   {
      eosio::name member;
      eosio::asset balance;
      member_status_type status;

      uint64_t primary_key() const { return member.value; }
   };
   using members_table_type = eosio::multi_index<"members"_n, member>;

   class members
   {
     private:
      eosio::name contract;
      members_table_type members_tb;

     public:
      members(eosio::name contract) : contract(contract), members_tb(contract, default_scope) {}

      void deposit(eosio::name member, eosio::asset quantity);
   };

}  // namespace eden
