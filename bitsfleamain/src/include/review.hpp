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
    struct [[eosio::table, eosio::contract("bitsfleamain")]] Reviewer
    {
        uint64_t id = 0;
        uint64_t uid = 0;
        name eosid;
        uint32_t voted_count = 0;
        time_point_sec create_time;
        time_point_sec last_active_time;

        uint64_t primary_key() const { return uid; }
        uint64_t by_rid() const { return id; }
        uint64_t by_eosid() const { return eosid.value; }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] ProductAudit
    {
        uint64_t id = 0;
        uint32_t pid = 0;
        uint64_t reviewer_uid = 0;
        /**
         * Whether the product delisted the audit
         */ 
        bool is_delisted = false;
        string review_details;
        time_point_sec review_time;

        uint64_t primary_key() const { return uint64_t(pid); }
        uint64_t by_audit_id() const { return id; }
        uint64_t by_reviewer_id() const { return reviewer_uid; }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Arbitration
    {
        uint64_t id = 0;
        uint64_t prosecutor = 0;
        string title;
        string resume;
        string detailed;
        time_point_sec create_tiem;
        uint64_t defendant = 0;
        string proof_content;
        string arbitration_results;
        uint64_t winner = 0;
        time_point_sec start_time;
        time_point_sec end_time;
        vector<uint64_t> reviewers;

        uint64_t primary_key() const { return id; }
        uint64_t by_prosecutor() const { return prosecutor; }
        uint64_t by_defendant() const { return defendant; }
    };

    typedef eosio::multi_index<"reviewers"_n, Reviewer,
        indexed_by< "byeosid"_n, const_mem_fun<Reviewer, uint64_t,  &Reviewer::by_eosid> >,
        indexed_by< "reviewerid"_n, const_mem_fun<Reviewer, uint64_t,  &Reviewer::by_rid> >
    > reviewer_index;

    typedef eosio::multi_index<"proaudits"_n, ProductAudit,
        indexed_by< "auditid"_n, const_mem_fun<ProductAudit, uint64_t,  &ProductAudit::by_audit_id> >,
        indexed_by< "reviewerid"_n, const_mem_fun<ProductAudit, uint64_t,  &ProductAudit::by_reviewer_id> >
    > proaudit_index;

    typedef eosio::multi_index<"arbitrations"_n, Arbitration,
        indexed_by< "prosecutor"_n, const_mem_fun<Arbitration, uint64_t,  &Arbitration::by_prosecutor> >,
        indexed_by< "defendant"_n, const_mem_fun<Arbitration, uint64_t,  &Arbitration::by_defendant> >
    > arbitration_index;
}