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

        //init Category
        string v[] = {"数码","玩具","美妆","居家","服装","图书","其他"};
        Category_index category_table( _self, _self.value);
        for( auto& item : v) {
            category_table.emplace( _self, [&](auto& c){
                auto id = category_table.available_primary_key();
                c.id = uint16_t(id == 0 ? id+1 : id);
                c.view = item;
            });
        }

        coin_index coin_table( _self, _self.value );
         //create FMP
        coin_table.emplace( _self, [&](auto& c){
            c.sym = FMP;
            c.fee = asset(0, FMP);
        });
        create( _self, asset( 10000000000000, FMP ), 30 );
        //create SYS
        coin_table.emplace( _self, [&](auto& c){
            c.sym = SYS;
            c.fee = asset(0, SYS);
        });
        symbol eos = symbol(symbol_code("EOS"), 4);
        if( SYS != eos ) {
            //EOS
            coin_table.emplace( _self, [&](auto& c){
                c.sym = eos;
                c.fee = asset(1000, eos);
                c.is_out = true;
            });
            create( _self, asset( 10000000000000, eos ), 0 );
        }
        // BTS
        symbol BTS = symbol(symbol_code("BTS"), 5);
        coin_table.emplace( _self, [&](auto& c){
            c.sym = BTS;
            c.fee = asset(100000, BTS);
            c.is_out = true;
        });
        create( _self, asset( 100000000000000, BTS ), 0 );

        // NULS
        symbol NULS = symbol(symbol_code("NULS"),8);
        coin_table.emplace( _self, [&](auto& c){
            c.sym = NULS;
            c.fee = asset(50000000, NULS);
            c.is_out = true;
        });
        create( _self, asset(10000000000000000, NULS ), 0 );

        // ETH
        symbol ETH = symbol(symbol_code("ETH"), 8);
        coin_table.emplace( _self, [&](auto& c){
            c.sym = ETH;
            c.fee = asset(1000000, ETH);
            c.is_out = true;
        });
        create( _self, asset(10000000000000000, ETH ), 0 );

        //USDT
        symbol USDT = symbol(symbol_code("USDT"), 8);
        coin_table.emplace( _self, [&](auto& c){
            c.sym = USDT;
            c.fee = asset(130000000, USDT);
            c.is_out = true;
        });
        create( _self, asset(10000000000000000, USDT ), 0 );

        // CNY
        symbol CNY = symbol(symbol_code("CNY"), 4);
        coin_table.emplace( _self, [&](auto& c){
            c.sym = CNY;
            c.fee = asset(20000, CNY);
            c.is_out = true;
        });
        create( _self, asset(10000000000000, CNY ), 0 );
    }

    void bitsfleamain::reset()
    {
        require_auth( _self );
        for_each( _global.income.begin(), _global.income.end(), [&](auto& a){
            a.amount = 300000000;
        });
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