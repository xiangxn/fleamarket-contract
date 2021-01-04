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
        _global.encrypt_key = str_to_pub("EOS6xf3J5cLnEzTvsXjSncremGV7s1CBk8S3ovLnimuLs9q2k5BKz");

        //init Category
        string v[] = {"数码","玩具","美妆","居家","服装","图书","虚拟","其他"};
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
        create( _self, asset( 10000000000000, FMP ), 35 );
        //create SYS
        coin_table.emplace( _self, [&](auto& c){
            c.sym = SYS;
            c.fee = asset(0, SYS);
        });
        if( SYS != EOS ) {
            //EOS
            coin_table.emplace( _self, [&](auto& c){
                c.sym = EOS;
                c.fee = asset(1000, EOS);
                c.is_out = true;
            });
            // create( _self, asset( 10000000000000, EOS ), 0 );
        }
        // BTS
        coin_table.emplace( _self, [&](auto& c){
            c.sym = BTS;
            c.fee = asset(100000, BTS);
            c.is_out = true;
        });
        create( _self, asset( 100000000000000, BTS ), 0 );

        // NULS
        coin_table.emplace( _self, [&](auto& c){
            c.sym = NULS;
            c.fee = asset(50000000, NULS);
            c.is_out = true;
        });
        create( _self, asset(10000000000000000, NULS ), 0 );

        // ETH
        // symbol ETH = symbol(symbol_code("ETH"), 8);
        // coin_table.emplace( _self, [&](auto& c){
        //     c.sym = ETH;
        //     c.fee = asset(1000000, ETH);
        //     c.is_out = true;
        // });
        // create( _self, asset(10000000000000000, ETH ), 0 );

        //USDT
        coin_table.emplace( _self, [&](auto& c){
            c.sym = USDT;
            c.fee = asset(130000000, USDT);
            c.is_out = true;
        });
        create( _self, asset(10000000000000000, USDT ), 0 );

        // CNY
        // symbol CNY = symbol(symbol_code("CNY"), 4);
        // coin_table.emplace( _self, [&](auto& c){
        //     c.sym = CNY;
        //     c.fee = asset(20000, CNY);
        //     c.is_out = true;
        // });
        // create( _self, asset(10000000000000, CNY ), 0 );
    }

    // void bitsfleamain::reset()
    // {
    //     require_auth( _self );

    // }

    void bitsfleamain::update(const string& para)
    {
        require_auth( _self );
        
    }

    void bitsfleamain::cleanscope( const name& n)
    {
        require_auth( _self );
        balances b_table( _self, n.value);
        clear_table(b_table);
    }

    void bitsfleamain::OnError( const onerror& error )
    {
        
    }

    void bitsfleamain::AddTableLog(const name& table, OpType type, uint32_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? 1 : t.id;
            t.table = table;
            t.type = type;
            t.primary = std::to_string(primary);
        });
    }

    
    void bitsfleamain::AddTableLog(const name& table, OpType type, uint64_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? 1 : t.id;
            t.table = table;
            t.type = type;
            t.primary = std::to_string(primary);
        });
    }

    void bitsfleamain::AddTableLog(const name& table, OpType type, uint128_t primary)
    {
        _tablelog_table.emplace( _self, [&](auto& t){
            t.id = _tablelog_table.available_primary_key();
            t.id = t.id == 0 ? 1 : t.id;
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