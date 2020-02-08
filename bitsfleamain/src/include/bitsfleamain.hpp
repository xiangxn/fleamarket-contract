#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>
#include <string>

#include "global.hpp"
#include "types.hpp"

#include "token/token_stats.hpp"
#include "token/token_balance.hpp"

#include "user.hpp"
#include "product.hpp"
#include "review.hpp"

using namespace eosio;

namespace rareteam {

    CONTRACT bitsfleamain : public contract {
    private:
        /**
         * Whether it has been initialized
         */ 
        bool                    _is_init = false;
        flea_global_singleton   _global_table;
        global                  _global;
        user_index              _user_table;

        global get_default_global() {
            global g;
            return g;
        }

        /*****token*****/
        void sub_balance( name owner, asset value );
        void add_balance( name owner, asset value, name ram_payer );
        /*****token end*****/

        void BidAuction( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const asset& price );
        void PayOrder( uint128_t order_id, const asset& quantity );
        void EndOrder( const Order& order );
        void Refund( const Order& order );
        bool CheckReviewer( uint64_t reviewer_uid, bool is_new = false );
        void SubCredit( const User& user, uint32_t value, bool cancel_reviewer = false );
        void SubCredit( uint64_t user_uid, uint32_t value, bool cancel_reviewer = false );
        void AddCredit( const User& user, uint32_t value );
        void AddCredit( uint64_t user_uid, uint32_t value );
        void RewardReferrer( const name& referrer );
        void Settle( const Order& order, const User& seller, const User& buyer);
        bool IsLockUser( uint64_t user_uid );
        bool IsLockUser( const User& user );
        bool CheckSymbol( const symbol& symbol );
        void PayCoin( const string& stroid, const Order& order, const User& seller, const User& buyer, const asset& seller_income, const asset& referrer_income, const name& contract);
        void Withdraw( const name& user_eosid, const asset& quantity );

    public:
        bitsfleamain( name receiver, name code, datastream<const char*> ds );
        ~bitsfleamain() {
            if( _is_init ){
                _global_table.set( _global, _self );
            }
        }

        /*****fleamain management*****/
        ACTION init();
        ACTION reset();
        ACTION test(const string& para);

        void OnError( const onerror& error );
        /*****fleamain management end*****/

        /*****Token management*****/
        ACTION create( name issuer, asset maximum_supply, int16_t team );
        ACTION issue( name to, asset quantity, string memo );
        ACTION retire( asset quantity, string memo );
        ACTION transfer( const name& from, const name& to, const asset& quantity, const string& memo );
        ACTION claim( name to, asset quantity );
        ACTION closetoken( name owner, const asset& symbol );
        /*****Token management end*****/

        /********platform*********/
        ACTION reguser( const name& eosid, const string& nickname, const checksum256& phone_hash, const string& phone_encrypt, uint64_t referrer );
        ACTION appreviewer( uint64_t uid, const name& eosid );
        ACTION votereviewer( uint64_t voter_uid, const name& voter_eosid, uint64_t reviewer_uid, bool is_support );
        ACTION publish( uint64_t uid, const Product& product, const ProductAuction& pa );
        ACTION review( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t pid, bool is_delisted, string& memo );
        ACTION pulloff( uint64_t seller_uid, const name& seller_eosid, uint32_t pid );
        ACTION placeorder( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid);
        ACTION shipment( uint64_t seller_uid, const name& seller_eosid, uint128_t order_id, const string& number );
        ACTION reshipment( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& number );
        ACTION conreceipt( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id );
        ACTION reconreceipt( uint64_t seller_uid, const name& seller_eosid, uint128_t order_id );
        ACTION returns( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& reasons );
        ACTION applyarbit( uint64_t plaintiff_uid, const name& plaintiff_eosid, const Arbitration& arbitration );
        ACTION inarbit( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t arbit_id );
        ACTION updatearbit( const Arbitration& arbit );
        ACTION deferreceipt( uint64_t user_uid, const name& user_eosid, uint128_t order_id );
        ACTION deferreturn( uint64_t user_uid, const name& user_eosid, uint128_t order_id );
        ACTION startsync();
        ACTION endsync();
        ACTION closesettle( uint64_t os_id );
        ACTION bindaddr( uint64_t uid, const name& user_eosid, const symbol& sym, const string& addr );
        
        
        /********platform End*****/

        // notify
        [[eosio::on_notify("eosio.token::transfer")]]
        void OnEOSTransfer( const name& from, const name& to, const asset& quantity, const string& memo );
        //[[eosio::on_notify("bitsfleamain::transfer")]]
        void OnMyTransfer( const name& from, const name& to, const asset& quantity, const string& memo );


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