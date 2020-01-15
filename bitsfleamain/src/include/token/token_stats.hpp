#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace rareteam {

   struct [[eosio::table, eosio::contract("bitsfleamain")]] TokenStat {
      asset supply;
      asset max_supply;
      name issuer;
      asset team;
      asset team_claim;

      uint64_t primary_key()const { return supply.symbol.code().raw(); }

   };

   typedef eosio::multi_index< "stat"_n, TokenStat > tokenStats;
}