#include "include/bitsfleamain.hpp"
#include "include/token/token_stats.hpp"
#include "include/token/token_balance.hpp"



namespace rareteam {

   void bitsfleamain::create( name issuer, asset maximum_supply, int16_t team )
   {
      require_auth( _self );

      auto sym = maximum_supply.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( maximum_supply.is_valid(), "invalid supply");
      check( maximum_supply.amount > 0, "max-supply must be positive");
      check( team >= 0 && team <= 100, "invalid team share" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing == statstable.end(), "token with symbol already exists" );

      statstable.emplace( _self, [&]( auto& s ) {
         s.supply.symbol        = maximum_supply.symbol;
         s.max_supply           = maximum_supply;
         s.issuer               = issuer;
         s.team                 = maximum_supply * team / 100;
         s.team_claim.symbol    = maximum_supply.symbol;
      });
   }

   void bitsfleamain::claim( name to, asset quantity )
   {
      require_auth( _self );

      auto sym = quantity.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must claim positive quantity" );
      check( sym == FMP, "must claim FMP" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing != statstable.end(), "token with symbol does not exist, create token before claim" );
      
      const auto& st = *existing;
      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( quantity.amount <= st.team.amount - st.team_claim.amount, "quantity exceeds available team");

      //The team has a total of linear unlocks
      asset one = st.team / int64_t(_global.team_unlock_time);
      uint32_t days = uint32_t((current_time_point().sec_since_epoch() - _global.project_start_time.sec_since_epoch()) / 86400);
      asset claim_ava = (one * days) - st.team_claim;
      check( quantity.amount <= claim_ava.amount, "quantity exceeds available team");

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply += quantity;
         s.team_claim += quantity;
      });
      add_balance( st.issuer, quantity, st.issuer );
      if( to != st.issuer ) {
         action( permission_level{st.issuer, "active"_n}, "bitsfleamain"_n, "transfer"_n,
                std::make_tuple( st.issuer, to, quantity, string("team claim") )
         ).send();
      }
   }

   void bitsfleamain::issue( name to, asset quantity, string memo )
   {
      auto sym = quantity.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( memo.size() <= 256, "memo has more than 256 bytes" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
      const auto& st = *existing;

      require_auth( st.issuer );
      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must issue positive quantity" );

      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( quantity.amount <= (st.max_supply.amount - st.supply.amount) - (st.team.amount - st.team_claim.amount) , "quantity exceeds available supply");

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply += quantity;
      });

      add_balance( st.issuer, quantity, st.issuer );

      if( to != st.issuer ) {
         action( permission_level{st.issuer, "active"_n}, "bitsfleamain"_n, "transfer"_n,
                std::make_tuple( st.issuer, to, quantity, memo )
         ).send();
      }
   }

   void bitsfleamain::retire( asset quantity, string memo )
   {
      auto sym = quantity.symbol;
      check( sym.is_valid(), "invalid symbol name" );
      check( memo.size() <= 256, "memo has more than 256 bytes" );

      tokenStats statstable( _self, sym.code().raw() );
      auto existing = statstable.find( sym.code().raw() );
      check( existing != statstable.end(), "token with symbol does not exist" );
      const auto& st = *existing;

      require_auth( st.issuer );
      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must retire positive quantity" );

      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

      statstable.modify( st, same_payer, [&]( auto& s ) {
         s.supply -= quantity;
      });

      sub_balance( st.issuer, quantity );
   }

   void bitsfleamain::transfer( name from, name to, asset quantity, string memo )
   {
      check( from != to, "cannot transfer to self" );
      require_auth( from );
      check( is_account( to ), "to account does not exist");
      auto sym = quantity.symbol.code();
      tokenStats statstable( _self, sym.raw() );
      const auto& st = statstable.get( sym.raw() );

      require_recipient( from );
      require_recipient( to );

      check( quantity.is_valid(), "invalid quantity" );
      check( quantity.amount > 0, "must transfer positive quantity for mg token" );
      check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
      check( memo.size() <= 256, "memo has more than 256 bytes" );

      auto payer = has_auth( to ) ? to : from;

      sub_balance( from, quantity );
      add_balance( to, quantity, payer );
   }

   void bitsfleamain::closetoken( name owner, const asset& symbol )
   {
      auto auth = has_auth( owner ) ? owner : _self;
      require_auth( auth );
      balances acnts( _self, owner.value );
      auto it = acnts.find( symbol.symbol.code().raw() );
      check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
      check( it->balance.amount == 0, "Cannot close because the balance is not zero." );
      acnts.erase( it );
   }

   void bitsfleamain::sub_balance( name owner, asset value ) {
      balances from_acnts( _self, owner.value );

      const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
      check( from.balance.amount >= value.amount, "overdrawn balance" );

      from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
      if( value.symbol == FMP ){
         auto ueosid_idx = _user_table.get_index<"eosid"_n>();
         auto u_itr = ueosid_idx.find( owner.value );
         if( u_itr != ueosid_idx.end() ) {
            ueosid_idx.modify( u_itr, same_payer, [&](auto& u){
               u.point -= value;
            });
         }
      }
   }

   void bitsfleamain::add_balance( name owner, asset value, name ram_payer )
   {
      balances to_acnts( _self, owner.value );
      auto to = to_acnts.find( value.symbol.code().raw() );
      if( to == to_acnts.end() ) {
         to = to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
         });
      } else {
         to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
         });
      }
      if( value.symbol == FMP ){
         auto ueosid_idx = _user_table.get_index<"eosid"_n>();
         auto u_itr = ueosid_idx.find( owner.value );
         if( u_itr != ueosid_idx.end() ) {
            ueosid_idx.modify( u_itr, same_payer, [&](auto& u){
               u.point += value;
            });
         }
      }
   }
}