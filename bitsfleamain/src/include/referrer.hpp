#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <string>

#include "types.hpp"

using namespace std;
using namespace eosio;

namespace rareteam
{
    struct [[eosio::table, eosio::contract("bitsfleamain")]] Referrer
    {
        uint32_t id;
        uint64_t referrer_id;
        uint64_t uid;
        time_point_sec create_time;
        

        uint64_t primary_key() const { return uint64_t(id); }
        uint64_t by_referrer() const { return referrer_id; }
    };

    typedef eosio::multi_index<"referrers"_n, Referrer,
        indexed_by< "referrer"_n, const_mem_fun<Referrer, uint64_t,  &Referrer::by_referrer> >
    > referrer_index;

} // namespace egame
