#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"

#include "token.cpp"
#include "platform.cpp"

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