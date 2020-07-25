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
    _user_table(_self, _self.value),
    _tablelog_table(_self, _self.value)
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
        _global.encrypt_key = str_to_pub("EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo");

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

        clear_table(_user_table);

        coin_index coin_table( _self, _self.value );
        clear_table(coin_table);

        Category_index cate_table( _self, _self.value );
        clear_table(cate_table);

        product_index product_table( _self, _self.value );
        clear_table(product_table);

        proaudit_index proaudit_table( _self, _self.value );
        clear_table(proaudit_table);

        reviewer_index reviewer_table( _self, _self.value );
        clear_table(reviewer_table);

        balances rb_table( _self, name("reviewer1111").value);
        clear_table(rb_table);

        balances bb_table( _self, name("bitsflea1111").value);
        clear_table(bb_table);
        
        symbol eos = symbol(symbol_code("EOS"), 4);
        tokenStats eos_table( _self, eos.code().raw() );
        clear_table(eos_table);

        tokenStats fmp_table( _self, FMP.code().raw() );
        clear_table(fmp_table);

        symbol BTS = symbol(symbol_code("BTS"), 5);
        tokenStats bts_table( _self, BTS.code().raw() );
        clear_table(bts_table);

        symbol NULS = symbol(symbol_code("NULS"),8);
        tokenStats nuls_table( _self, NULS.code().raw() );
        clear_table(nuls_table);

        symbol ETH = symbol(symbol_code("ETH"), 8);
        tokenStats eth_table( _self, ETH.code().raw() );
        clear_table(eth_table);

        symbol USDT = symbol(symbol_code("USDT"), 8);
        tokenStats usdt_table( _self, USDT.code().raw() );
        clear_table(usdt_table);

        symbol CNY = symbol(symbol_code("CNY"), 4);
        tokenStats cny_table( _self, CNY.code().raw() );
        clear_table(cny_table);

        _global_table.remove();
    }

    void bitsfleamain::test(const string& para)
    {
        // string str = "05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492";
        // auto hash = hex_to_sha256(str);
        // string result = sha256_to_hex(hash);
        //check(false,result);
        //set_zero( hash );
        //check(is_zero( hash ),"is_zero Error");
        //result = sha256_to_hex(hash);
        //check(false,result);
        //auto num = get_uint64(hash);
        
        //check(false,result);

        name n = "reviewer1112"_n;
        print(n.value);
    }

    void bitsfleamain::OnError( const onerror& error )
    {
        
    }

    void bitsfleamain::AddTableLog(const name& table, OpType type, uint32_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? t.id + 1 : t.id;
            t.table = table;
            t.type = type;
            t.primary = std::to_string(primary);
        });
    }

    
    void bitsfleamain::AddTableLog(const name& table, OpType type, uint64_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? t.id + 1 : t.id;
            t.table = table;
            t.type = type;
            t.primary = std::to_string(primary);
        });
    }

    void bitsfleamain::AddTableLog(const name& table, OpType type, uint128_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? t.id + 1 : t.id;
            t.table = table;
            t.type = type;
            t.primary = uint128ToString(primary);
        });
    }

    void bitsfleamain::DeleteTableLog(uint64_t id)
    {
        auto itr = _tablelog_table.begin();
        while ( itr != _tablelog_table.end() && itr->id <= id )
        {
            itr = _tablelog_table.erase(itr);
        }
    }

    void bitsfleamain::deletelog( uint64_t id)
    {
        require_auth( _self );
        DeleteTableLog(id);
    }

    

    
}