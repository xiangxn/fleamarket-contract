#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;

namespace rareteam {

   #define FMP symbol(symbol_code("FMP"), 4)
   #define SYS symbol(symbol_code("EOS"), 4)

   #define FLEA_PLATFORM ("bitsfleamain"_n)
   #define NAME_EOSIO_TOKEN name("eosio.token")

   #define ACTIVE_PERMISSION name("active")
   #define ACTION_NAME_TRANSFER name("transfer")
   #define ACTION_NAME_LOCKUSER name("lockuser")

   #define EOS_DISPATCH( TYPE, MEMBERS ) \
   extern "C" { \
      void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
         if( code == name("eosio.token").value && action == name("transfer").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::deposit ); \
         } else if( code == name("eosio").value && action == name("onerror").value ) { \
            eosio::execute_action( eosio::name(receiver), eosio::name(code), &TYPE::OnError ); \
         } else if( code == receiver ) { \
            switch( action ) { \
               EOSIO_DISPATCH_HELPER( TYPE, MEMBERS ) \
               default: \
                  check( false, "Invalid action request" ); \
                  break; \
            } \
            /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
         } \
      } \
   } \

}
