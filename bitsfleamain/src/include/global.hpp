#pragma once

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <string>

#include "types.hpp"

using namespace std;

namespace rareteam {

    struct [[eosio::table, eosio::contract("bitsfleamain")]] global {
        /**
         * The total amount of users in the platform.
         */
        uint32_t total_users = 0;

        /************************Points related************************/
        /**
         * Credit limit for participation in referrals
         */ 
        asset ref_limit_fmp = asset( 5000000, FMP );
        /**
         * the referral commission
         */ 
        uint32_t ref_commission_scale = 1;
        /**
         * Points gift once by the system at the time of referral
         */ 
        asset ref_sys_gift = asset( 10, FMP );
        /**
         * Referral gift pool
         */ 
        asset ref_pool = asset( 0, FMP );
        /************************Points related End************************/

        /*******************credit related***************/
        /**
         * Basic credit score
         */ 
        int32_t credit_base_score = 500;

        /**
         * Lower credit limit for reviewers
         */
        int32_t credit_reviewer_limit = 500;
        /*******************credit related End***********/

        /*******************reviewer related**************************/
        /**
         * Minimum number of reviewers
         */ 
        uint32_t review_min_count = 3;
        /**
         * Maximum number of reviewers
         */ 
        uint32_t review_max_count = 3000;
        /**
         * Reviewer daily base salary
         */ 
        asset review_salary_base = asset( 100000, FMP );
        /**
         * Review the salary of a product
         */ 
        asset review_salary_product = asset( 20000, FMP );
        /**
         * Reviewer's salary for each dispute
         */ 
        asset review_salary_dispute = asset( 100000, FMP );
        /*******************reviewer related End**************************/
        /**
         * Project profit list
         */ 
        vector<asset> income;
        /**
         * the project start time.
         */ 
        time_point_sec project_start_time = time_point_sec(0);
        /**
         * Team FMP share unlock days
         */ 
        uint32_t team_unlock_time = 365;
    };

   typedef eosio::singleton< "global"_n, global> flea_global_singleton;
}