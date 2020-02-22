#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"

#include "token.cpp"
#include "platform.cpp"
#include "market.cpp"

using namespace eosio;

namespace rareteam {

    bitsfleamain::bitsfleamain( name receiver, name code, datastream<const char*> ds ): eosio::contract( receiver, code, ds ),
    _global_table(_self, _self.value),
    _user_table(_self, _self.value)
    {
        if( _global_table.exists() ){
            _is_init = true;
            _global = _global_table.get();
        }
    }

    void bitsfleamain::init() {
        require_auth( _self );
        if( _is_init ) return;

        _is_init = true;
        _global = get_default_global();
        _global.project_start_time = time_point_sec(current_time_point().sec_since_epoch());
        _global.ref_pool = asset( 1000000000000, FMP );
        _global.transaction_pool = asset( 3500000000000, FMP );
        _global.salary_pool = asset( 2000000000000, FMP );

        transaction trx;
         //create FMP
        _global.support_coin.push_back( FMP );
        action(permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, "create"_n,
            std::make_tuple( _self, asset( 10000000000000, FMP ), 30 )
        ).send();
        //create SYS
        _global.support_coin.push_back( SYS );
        symbol eos = symbol(symbol_code("EOS"), 4);
        if( SYS != eos ) {
            //EOS
            _global.support_coin.push_back( eos );
            action a1 = action(permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, "create"_n,
                std::make_tuple( _self, asset( 10000000000000, eos ), 0 )
            );
            trx.actions.emplace_back( a1 );
        }
        // BTS
        symbol BTS = symbol(symbol_code("BTS"), 5);
        _global.support_coin.push_back( BTS );
        action a2 = action(permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, "create"_n,
            std::make_tuple( _self, asset( 36000000000000, BTS ), 0 )
        );
        trx.actions.emplace_back( a2 );

        // NULS
        symbol NULS = symbol(symbol_code("NULS"),8);
        _global.support_coin.push_back( NULS );
        action a3 = action(permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, "create"_n,
            std::make_tuple( _self, asset(10000000000000000, NULS ), 0 )
        );
        trx.actions.emplace_back( a3 );


        trx.delay_sec = 2;
        trx.send( (uint128_t(("init"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
    }

    void bitsfleamain::reset()
    {
        require_auth( _self );
    }

    void bitsfleamain::test(const string& para)
    {
        string str = "05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492";
        auto hash = hex_to_sha256(str);
        string result = sha256_to_hex(hash);
        //check(false,result);
        //set_zero( hash );
        //check(is_zero( hash ),"is_zero Error");
        //result = sha256_to_hex(hash);
        //check(false,result);
        //auto num = get_uint64(hash);
        
        //check(false,result);
    }

    void bitsfleamain::OnError( const onerror& error )
    {
        
    }

    


    

    
}