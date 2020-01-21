#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"
#include "../../tools/include/tool.hpp"

using namespace eosio;

namespace rareteam {

    void bitsfleamain::reguser( const name& eosid, const string& nickname, const checksum256& phone_hash, const string& phone_encrypt, uint64_t referrer )
    {
        require_auth( _self );

        check( is_account(eosid), "Invalid account eosid" );
        check( nickname.length() <= 20, "Nickname is too long" );
        check( is_zero(phone_hash) == false, "invalid phone hash" );
        if( referrer > 0 ) {
            auto ref_itr = _user_table.find( referrer );
            check( ref_itr != _user_table.end(), "invalid referrer id" );
        }

        auto phone_idx = _user_table.get_index<"phone"_n>();
        auto phone_itr = phone_idx.find( phone_hash );
        check( phone_itr == phone_idx.end(), "Phone number is already registered" );

        auto eos_idx = _user_table.get_index<"eosid"_n>();
        auto eos_itr = eos_idx.find( eosid.value );
        check( eos_itr == eos_idx.end(), "eos id is already registered" );
        
        auto u_itr = _user_table.emplace( _self, [&]( auto& u){
            u.uid = _user_table.available_primary_key();
            u.eosid = eosid;
            u.nickname = nickname;
            u.phone_hash = phone_hash;
            u.phone_encrypt = phone_encrypt;
            u.credit_value = _global.credit_base_score;
            u.last_active_time = time_point_sec(current_time_point().sec_since_epoch());
            if( referrer > 0 ) {
                u.referrer = referrer;
            }
        });

        if( referrer > 0 ) {
            referrer_index ref_table( _self, _self.value );
            ref_table.emplace( _self, [&]( auto& r ){
                r.id = ref_table.available_primary_key();
                r.referrer_id = referrer;
                r.uid = u_itr->uid;
                r.create_time = time_point_sec(current_time_point().sec_since_epoch());
            });
            auto ru_itr = _user_table.find( referrer );
            _user_table.modify( ru_itr, same_payer, [&]( auto& ru ) {
                ru.referral_total += 1;
            });
        }
        _global.total_users += 1;
    }

    void bitsfleamain::applyreview( uint64_t uid, const name& eosid )
    {
        require_auth( eosid );

        check( is_account(eosid), "Invalid account eosid" );
        auto& user = _user_table.get( uid, "Invalid account id" );
        check( user.credit_value >= _global.credit_reviewer_limit, "Insufficient credit value" );

        reviewer_index rev_table( _self, _self.value );
        auto itr = rev_table.find( uid );
        check( itr == rev_table.end(), "Already reviewer" );

        auto rev_itr = rev_table.emplace( _self, [&]( auto& r ){
            r.id = rev_table.available_primary_key();
            r.uid = uid;
            r.eosid = eosid;
            r.voted_count = 0;
            r.create_time = time_point_sec(current_time_point().sec_since_epoch());
            r.last_active_time = time_point_sec(current_time_point().sec_since_epoch());
        });
    }

    void bitsfleamain::votereviewer( uint64_t voter_uid, const name& voter_eosid, uint64_t reviewer_uid, bool is_support )
    {
        require_auth( voter_eosid );
        reviewer_index rev_table( _self, _self.value );
        auto itr = rev_table.find( reviewer_uid );
        check( itr != rev_table.end(), "The reviewer uid is not a reviewer" );

        rev_table.modify( itr, same_payer, [&]( auto& r ){
            if( is_support ){
                r.voted_count += 1;
            }else{
                r.voted_count -= 1;
            }
            if( r.voted_count < 0 ) r.voted_count = 0;
        });
        //TODO: point logic

    }
    

    
}