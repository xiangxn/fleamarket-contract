#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;

namespace rareteam {

   #define N_CONTRACT(_NAME_) class [[eosio::contract(#_NAME_)]]
   #define N_CONTRACT_TABLE(_CNAME_) struct [[ eosio::table, eosio::contract(#_CNAME_) ]]
   #define N_TABLE(_NAME_) struct [[eosio::table(#_NAME_)]]
   #define N_ON_NOTIFY(_NAME_) [[eosio::on_notify(#_NAME_)]]

   #define FMP symbol(symbol_code("FMP"), 4)
   #define SYS symbol(symbol_code("BOS"), 4)

   #define FLEA_PLATFORM ("bitsfleamain"_n)
   #define NAME_EOSIO_TOKEN name("eosio.token")

   #define ACTIVE_PERMISSION name("active")
   #define ACTION_NAME_TRANSFER name("transfer")
   #define ACTION_NAME_LOCKUSER name("lockuser")

}
