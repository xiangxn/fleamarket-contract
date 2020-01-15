#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"

#include "token.cpp"

using namespace eosio;

namespace rareteam {

    fleamain::fleamain( name receiver, name code, datastream<const char*> ds ): eosio::contract( receiver, code, ds ),
    _global_table(_self, _self.value)
    {
        _global = _global_table.exists() ? _global_table.get() : get_default_global();
    }

    void fleamain::OnError( const onerror& error )
    {
        
    }


    void fleamain::deposit( name from, name to, asset quantity, string memo )
    {
    }
    void fleamain::withdraw( const name& to, const asset& quantity )
    {
    }

    void fleamain::reset()
    {

    }

    
}

EOS_DISPATCH( rareteam::fleamain, (deposit)
                     (withdraw)
                     (reset) 
                     /*****token*****/
                     (create)
                     (issue)
                     (retire)
                     (transfer)
                     (claim)
                     (closetoken)
                     /*****token end*****/
                     )