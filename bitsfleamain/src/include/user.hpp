#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>
#include <string>

#include "types.hpp"

using namespace std;
using namespace eosio;

namespace rareteam
{
    enum UserStatus : uint8_t {
        NONE = 0,
        LOCK
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] User
    {
        uint64_t uid;
        name eosid;
        uint8_t status = UserStatus::NONE;
        bool is_reviewer = false;
        bool is_data_node = false;

        string nickname;
        string head;
        checksum256 phone_hash;
        string phone_encrypt;
        uint64_t referrer = 0;
        uint32_t credit_value = 0;
        public_key auth_key;
        
        time_point_sec last_active_time;

        uint32_t posts_total = 0;
        uint32_t sell_total = 0;
        uint32_t buy_total = 0;
        //uint32_t favorite_total = 0;
        uint32_t referral_total = 0;

        asset point = asset( 0, FMP );

        uint64_t primary_key() const { return uid; }
        uint64_t by_eosid() const { return eosid.value; }
        checksum256 by_phone() const { return phone_hash; }

    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] OtherAddr
    {
        uint32_t id;
        uint64_t uid;
        symbol coin_type;
        string addr = "";

        uint64_t primary_key() const { return uint64_t(id); }
        uint64_t by_uid() const { return uid; }
    };

    typedef eosio::multi_index<"users"_n, User,
        indexed_by< "eosid"_n, const_mem_fun<User, uint64_t,  &User::by_eosid> >,
        indexed_by< "phone"_n, const_mem_fun<User, checksum256,  &User::by_phone> >
    > user_index;

    typedef eosio::multi_index<"otheraddr"_n, OtherAddr,
        indexed_by< "byuid"_n, const_mem_fun<OtherAddr, uint64_t, &OtherAddr::by_uid> >
    > otheraddr_index;

} // namespace egame
