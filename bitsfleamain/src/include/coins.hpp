#pragma once

#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

namespace rareteam
{
    struct [[eosio::table, eosio::contract("bitsfleamain")]] Coin
    {
        symbol sym;
        asset fee;
        bool is_out = false;

        uint64_t primary_key() const { return sym.code().raw(); }
    };

    typedef eosio::multi_index< "coins"_n, Coin > coin_index;
}