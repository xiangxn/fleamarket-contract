#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace rareteam {

   struct [[eosio::table, eosio::contract("bitsfleamain")]] Balance {
      asset balance;

      uint64_t primary_key()const { return balance.symbol.code().raw(); }
   };

   typedef eosio::multi_index< "accounts"_n, Balance > balances;
}