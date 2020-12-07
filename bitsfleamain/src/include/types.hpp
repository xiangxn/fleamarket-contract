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
   #define EOS symbol(symbol_code("EOS"), 4)
   #define BTS symbol(symbol_code("BTS"), 5)
   #define NULS symbol(symbol_code("NULS"), 8)
   #define USDT symbol(symbol_code("USDT"), 8)
   #define ETH symbol(symbol_code("ETH"), 8)
   #define CNY symbol(symbol_code("CNY"), 4)

   #define NAME_EOSIO_TOKEN name("eosio.token")
   #define NAME_BOSIBC_TOKEN name("bosibc.io")

   #define ACTIVE_PERMISSION name("active")
   #define ACTION_NAME_TRANSFER name("transfer")
   #define ACTION_NAME_LOCKUSER name("lockuser")

}
