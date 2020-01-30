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

    enum ArbitStatus : uint32_t {
        AS_APPLY = 0,
        AS_WAIT = 100,
        AS_PROCESSING = 200,
        AS_COMPLETED = 300
    };

    enum ArbitType : uint32_t {
        AT_ORDER = 0,
        AT_COMPLAINT = 100,
        AT_PRODUCT = 200
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Reviewer
    {
        uint64_t id = 0;
        uint64_t uid = 0;
        name eosid;
        uint32_t voted_count = 0;
        time_point_sec create_time;
        time_point_sec last_active_time;
        vector<uint64_t> voter_approve;
        vector<uint64_t> voter_against;

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
        uint32_t id = 0;
        uint64_t plaintiff = 0;
        uint32_t pid = 0;
        uint128_t order_id = 0;
        uint32_t type = ArbitType::AT_ORDER;
        uint32_t status = ArbitStatus::AS_APPLY;
        string title;
        string resume;
        string detailed;
        time_point_sec create_time;
        uint64_t defendant = 0;
        string proof_content = "";
        string arbitration_results = "";
        uint64_t winner = 0;
        time_point_sec start_time;
        time_point_sec end_time;
        vector<uint64_t> reviewers;

        uint64_t primary_key() const { return uint64_t(id); }
        uint64_t by_plaintiff() const { return plaintiff; }
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
        indexed_by< "plaintiff"_n, const_mem_fun<Arbitration, uint64_t,  &Arbitration::by_plaintiff> >,
        indexed_by< "defendant"_n, const_mem_fun<Arbitration, uint64_t,  &Arbitration::by_defendant> >
    > arbitration_index;
}