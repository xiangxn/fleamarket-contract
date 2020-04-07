#pragma once

#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;

namespace rareteam
{
    enum OpType : uint32_t {
        OT_INSERT = 0,
        OT_DELETE = 1,
        OT_UPDATE = 2
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] TableLog
    {
        uint64_t id = 0;
        name table = name(0);
        uint32_t type = OpType::OT_INSERT;
        string primary = "";
        

        uint64_t primary_key() const { return id; }
    };

    typedef eosio::multi_index< "tablelogs"_n, TableLog > tablelog_index;
}