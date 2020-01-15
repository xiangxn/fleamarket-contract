#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <string>

#include "global.hpp"
#include "data.hpp"

#include "token/token_stats.hpp"
#include "token/token_balance.hpp"

using namespace eosio;

namespace rareteam {

    class [[eosio::contract("bitsfleamain")]] fleamain : public contract {
    private:
        flea_global_singleton   _global_table;
        global                  _global;

        global get_default_global() {
            global g;
            return g;
        }

        /*****token*****/
        void sub_balance( name owner, asset value );
        void add_balance( name owner, asset value, name ram_payer );
        /*****token end*****/

    public:
        fleamain( name receiver, name code, datastream<const char*> ds );
        ~fleamain() {
            _global_table.set( _global, _self );
        }

        /*****deposit/withdraw*****/ 
        ACTION deposit( name from, name to, asset quantity, string memo );

        ACTION withdraw( const name& to, const asset& quantity );
        /*****deposit/withdraw end*****/

        /*****fleamain management*****/
        ACTION reset();

        void OnError( const onerror& error );
        /*****fleamain management end*****/

        /*****Token management*****/
        ACTION create( name issuer, asset maximum_supply, int16_t team );
        ACTION issue( name to, asset quantity, string memo );
        ACTION retire( asset quantity, string memo );
        ACTION transfer( name from, name to, asset quantity, string memo );
        ACTION claim( name to, asset quantity );
        ACTION closetoken( name owner, const asset& symbol );
        /*****Token management end*****/

    public:
        // static const user& GetUser( const name& tokenContractAccount, const name& account )
        // {
        //     user_index users( tokenContractAccount, tokenContractAccount.value );
        //     const auto& acc = users.get( account.value, "unable to find account" );
        //     return acc;
        // }

        // static const user FindUser( const name& tokenContractAccount, const name& account )
        // {
        //     user_index users( tokenContractAccount, tokenContractAccount.value );
        //     user u;
        //     auto aitr = users.find( account.value );
        //     if( aitr != users.end() ) {
        //         u = *aitr;
        //     }
        //     return u;
        // }
    
        // static void CheckUser( const name& tokenContractAccount, const name& account )
        // {
        //     user_index users( tokenContractAccount, tokenContractAccount.value );
        //     auto itr = users.find( account.value );
        //     eosio_assert( itr != users.end(), "unknown account" );
        //     eosio_assert( itr->status == 0, "this account has been locked" );
        //     eosio_assert( itr->game_id.value == 0, "already in the game" );
        // }


        /*****token*****/
        static asset GetSupply( name tokenContractAccount, symbol_code symCode )
        {
            tokenStats statstable( tokenContractAccount, symCode.raw() );
            const auto& st = statstable.get( symCode.raw() );
            return st.supply;
        }

        static asset GetBalance( name tokenContractAccount, name owner, symbol_code symCode )
        {
            balances accountstable( tokenContractAccount, owner.value );
            const auto& ac = accountstable.get( symCode.raw() );
            return ac.balance;
        }
        /*****token end*****/

    };
}