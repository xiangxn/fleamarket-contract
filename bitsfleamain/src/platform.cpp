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
        bool is_ref = false;
        if( referrer > 0 ) {
            auto& ref = _user_table.get( referrer, "invalid referrer id" );
            is_ref = ref.credit_value >= _global.credit_ref_limit;
            is_ref = IsLockUser( ref ) == false;
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
            if( referrer > 0 && is_ref ) {
                u.referrer = referrer;
            }
        });
        AddTableLog("users"_n, OpType::OT_INSERT, u_itr->uid);

        if( referrer > 0 && is_ref ) {
            auto ru_itr = _user_table.find( referrer );
            _user_table.modify( ru_itr, same_payer, [&]( auto& ru ) {
                ru.referral_total += 1;
            });
            AddTableLog("users"_n, OpType::OT_UPDATE, ru_itr->uid);
            AddCredit( referrer, 1 );
            //Reward Referrer Points
            RewardReferrer( ru_itr->eosid );
        }
        _global.total_users += 1;
    }

    bool bitsfleamain::CheckReviewer( uint64_t reviewer_uid, bool is_new )
    {
        auto& user = _user_table.get( reviewer_uid, "Invalid account id" );
        check( IsLockUser( user ) == false, "Account is locked" );
        if( user.credit_value >= _global.credit_reviewer_limit ) {
            if( is_new )
                return true;
            else
                return user.is_reviewer;
        } else {
            _user_table.modify( user, same_payer, [&](auto& u){
                u.is_reviewer = false;
            });
            AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
            reviewer_index reviewer_table( _self, _self.value );
            auto itr = reviewer_table.find( reviewer_uid );
            if( itr != reviewer_table.end() ) {
                AddTableLog("reviewers"_n, OpType::OT_DELETE, itr->uid);
                reviewer_table.erase( itr );
            }
            return false;
        }
    }

    void bitsfleamain::appreviewer( uint64_t uid, const name& eosid )
    {
        require_auth( eosid );

        check( is_account(eosid), "Invalid account eosid" );
        check( CheckReviewer( uid, true ), "Insufficient credit value" );

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
            AddTableLog("reviewers"_n, OpType::OT_INSERT, r.uid);
        });
    }

    void bitsfleamain::votereviewer( uint64_t voter_uid, const name& voter_eosid, uint64_t reviewer_uid, bool is_support )
    {
        require_auth( voter_eosid );
        check( IsLockUser( voter_uid ) == false, "Account is locked" );
        auto& user = _user_table.get( reviewer_uid, "Invalid reviewer uid" );
        reviewer_index rev_table( _self, _self.value );
        auto& reviewer = rev_table.get( reviewer_uid, "The reviewer uid is not a reviewer" );
        if( is_support ) {
            check( reviewer.voter_approve.size() < 100, "maximum of 100 peoples can vote" );
        } else {
            check( reviewer.voter_against.size() < 100, "maximum of 100 peoples can vote" );
        }
        auto it = find_if(reviewer.voter_approve.begin(), reviewer.voter_approve.end(), [&](auto& u){ return u == voter_uid; });
        check( it == reviewer.voter_approve.end(), "You already voted" );
        auto it2 = find_if(reviewer.voter_against.begin(), reviewer.voter_against.end(), [&](auto& u){ return u == voter_uid; });
        check( it2 == reviewer.voter_against.end(), "You already voted" );

        rev_table.modify( reviewer, same_payer, [&]( auto& r ){
            if( is_support ){
                if( r.voted_count == 0 ){
                     _user_table.modify( user, same_payer, [&](auto& u){
                        u.is_reviewer = true;
                    });
                    AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
                }
                r.voted_count += 1;
                r.voter_approve.push_back( voter_uid );
            }else{
                r.voted_count -= 1;
                r.voter_against.push_back( voter_uid );
                if( r.voted_count == 0 ) {
                    //r.voted_count = 0;
                    _user_table.modify( user, same_payer, [&](auto& u){
                        u.is_reviewer = false;
                    });
                    AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
                }
            }
            AddTableLog("reviewers"_n, OpType::OT_UPDATE, reviewer.uid);
        });
        // point logic
        if( _global.gift_vote.amount > 0 && _global.transaction_pool.amount >= _global.gift_vote.amount ) {
            action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
                std::make_tuple( voter_eosid, _global.gift_vote, string("Reward voted") )
            ).send();
            _global.transaction_pool -= _global.gift_vote;
        }

    }

    void bitsfleamain::applyarbit( uint64_t plaintiff_uid, const name& plaintiff_eosid, const Arbitration& arbitration )
    {
        require_auth( plaintiff_eosid );

        auto& plaintiff = _user_table.get( plaintiff_uid, "Invalid account plaintiff uid" );
        auto& defendant = _user_table.get( arbitration.defendant, "Invalid account defendant uid" );

        product_index pro_table( _self, _self.value );
        auto pro_itr = pro_table.find( arbitration.pid );
        if( pro_itr != pro_table.end() ) {
            pro_table.modify( pro_itr, same_payer, [&](auto& p){
                p.status = ProductStatus::LOCKED;
            });
            AddTableLog("products"_n, OpType::OT_UPDATE, pro_itr->pid );
        }
        
        if( arbitration.type == ArbitType::AT_ORDER ) {
            //Usually initiated by the buyer
            order_index order_table( _self, _self.value );
            auto order_itr = order_table.find( arbitration.order_id );
            if( order_itr != order_table.end() ) {
                check( order_itr->status == OrderStatus::OS_PENDING_RECEIPT, "apply for arbitration until payment has been made and order is not completed");
                order_table.modify( order_itr, same_payer, [&](auto& o){
                    o.status = OrderStatus::OS_ARBITRATION;
                    AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
                });
            }
            //Usually initiated by the seller
            proreturn_index res_table( _self, _self.value );
            auto res_itr = res_table.find( arbitration.order_id );
            if( res_itr != res_table.end() ) {
                check( res_itr->status == ReturnStatus::RS_PENDING_RECEIPT, "Initiation of arbitration without confirmation of receipt" );
                res_table.modify( res_itr, same_payer, [&](auto& r){
                    r.status = ReturnStatus::RS_ARBITRATION;
                    AddTableLog( "returns"_n, OpType::OT_UPDATE, r.order_id );
                });
            }
        } else if( arbitration.type == ArbitType::AT_COMPLAINT ) { //complaint
            check( plaintiff.is_reviewer == false, "Complaints can only be initiated by ordinary users" );
        } else if ( arbitration.type == ArbitType::AT_PRODUCT ) {
            //TODO:Report product
        } else if ( arbitration.type == ArbitType::AT_ILLEGAL_INFO ) {
            //TODO:ILLEGAL INFO
        }

        arbitration_index arbit_table( _self, _self.value );
        arbit_table.emplace( _self, [&](auto& a){
            a.id = arbit_table.available_primary_key();
            a.plaintiff = plaintiff_uid;
            a.pid = arbitration.pid;
            a.order_id = arbitration.order_id;
            a.status = ArbitStatus::AS_APPLY;
            a.type = arbitration.type;
            a.title = arbitration.title;
            a.resume = arbitration.resume;
            a.detailed = arbitration.detailed;
            a.create_time = time_point_sec(current_time_point().sec_since_epoch());
            a.defendant = arbitration.defendant;
            a.proof_content = arbitration.proof_content;
            AddTableLog( "arbitrations"_n, OpType::OT_INSERT, a.id );
        });
    }

    void bitsfleamain::inarbit( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t arbit_id )
    {
        require_auth( reviewer_eosid );

        check( CheckReviewer( reviewer_uid ), "Invalid reviewer uid" );

        arbitration_index arbit_table( _self, _self.value );
        auto& arbit = arbit_table.get( uint64_t(arbit_id), "Invalid arbit id" );
        check( reviewer_uid != arbit.plaintiff, "You cannot be the plaintiff or defendant" );
        check( reviewer_uid != arbit.defendant, "You cannot be the plaintiff or defendant" );

        if( arbit.reviewers.size() < _global.review_min_count ) {
            arbit_table.modify( arbit, same_payer, [&](auto& a){
                a.reviewers.push_back( reviewer_uid );
                if( a.reviewers.size() >= _global.review_min_count ){
                    a.status = ArbitStatus::AS_PROCESSING;
                    a.start_time = time_point_sec(current_time_point().sec_since_epoch());
                } else {
                    a.status = ArbitStatus::AS_WAIT;
                }
            });
            AddTableLog( "arbitrations"_n, OpType::OT_UPDATE, arbit.id );
        }
    }

    void bitsfleamain::updatearbit( const Arbitration& arbit )
    {
        require_auth( _self );
        check( arbit.id >= 0, "Invalid arbit id" );
        arbitration_index arbit_table( _self, _self.value );
        auto& c_arbit = arbit_table.get( uint64_t(arbit.id), "Invalid arbit id" );
        check( c_arbit.reviewers.size() >= _global.review_min_count, "Invalid arbit update reviewer too little" );

        uint32_t auths = 0;
        uint32_t all = c_arbit.reviewers.size();
        for( uint32_t i=0; i<all; i++ ){
            auto& u = _user_table.get( c_arbit.reviewers[i], "Invalid uid" );
            if( has_auth( u.eosid ) ) auths += 1;
        }
        if( auths >= (all/2)+1 ) {
            arbit_table.modify( c_arbit, same_payer, [&](auto& a){
                a.end_time = time_point_sec(current_time_point().sec_since_epoch());
                a.status = ArbitStatus::AS_COMPLETED;
                a.arbitration_results = arbit.arbitration_results;
                a.winner = arbit.winner;
            });
            AddTableLog( "arbitrations"_n, OpType::OT_UPDATE, c_arbit.id );
            //order: Processing funds if there are orders
            if( c_arbit.type == ArbitType::AT_ORDER ) {
                time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
                order_index order_table( _self, _self.value );
                auto order_itr = order_table.find( c_arbit.order_id );
                if( order_itr != order_table.end() ) {
                    auto& order = (*order_itr);
                    proreturn_index repro_table( _self, _self.value );
                    auto repro_itr = repro_table.find( order.id );
                    if( c_arbit.winner == order.buyer_uid ) { // buyer win
                        if( repro_itr != repro_table.end() ) { //initiated by the seller
                            repro_table.modify( repro_itr, same_payer, [&](auto& r){
                                r.status = ReturnStatus::RS_COMPLETED;
                                AddTableLog( "returns"_n, OpType::OT_UPDATE, r.order_id );
                            });
                            order_table.modify( order_itr, same_payer, [&](auto& o){
                                o.status = OrderStatus::OS_CANCELLED;
                                o.end_time = current_time;
                                AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
                            });
                            //update product status
                            product_index pro_table( _self, _self.value );
                            auto& product = pro_table.get( order.pid, "Invalid product pid" );
                            pro_table.modify( product, same_payer, [&](auto& p){
                                p.status = ProductStatus::NORMAL;
                            });
                            AddTableLog("products"_n, OpType::OT_UPDATE, product.pid );
                            Refund( order );
                        } else { //initiated by the buyer
                            order_table.modify( order, same_payer, [&](auto& o){
                                o.status = OrderStatus::OS_RETURN;
                                AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
                            });
                            repro_table.emplace( _self, [&](auto& r){
                                r.id = repro_table.available_primary_key();
                                r.order_id = order.id;
                                r.pid = order.pid;
                                r.order_price = order.price;
                                r.status = ReturnStatus::RS_PENDING_SHIPMENT;
                                r.reasons = arbit.arbitration_results;
                                r.create_time = current_time;
                                r.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
                                AddTableLog( "returns"_n, OpType::OT_INSERT, r.order_id );
                            });
                        }
                        SubCredit( order.seller_uid, 100 );
                    } else if ( c_arbit.winner == order.seller_uid ) { //seller win
                        if( repro_itr != repro_table.end() ) { //initiated by the seller
                            order_table.modify( order_itr, same_payer, [&](auto& o){ //Redeliver
                                o.status = OrderStatus::OS_PENDING_SHIPMENT;
                                o.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
                                AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
                            });
                        } else { //initiated by the buyer
                            order_table.modify( order_itr, same_payer, [&](auto& o){
                                o.status = OrderStatus::OS_COMPLETED;
                                o.end_time = current_time;
                                AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
                            });
                            EndOrder( order );
                        }
                        SubCredit( order.buyer_uid, 100 );
                    }
                } else {
                    arbit_table.erase( c_arbit );
                }
            } else if ( c_arbit.type == ArbitType::AT_COMPLAINT ) {
                auto& plaintiff = _user_table.get( c_arbit.plaintiff, "Invalid plaintiff uid" );
                auto& defendant = _user_table.get( c_arbit.defendant, "Invalid defendant uid" );
                if( defendant.is_reviewer ) {
                    if( c_arbit.winner == plaintiff.uid ) { //plaintiff win
                        //sub defendant credit value and cancel reviewer
                        SubCredit( defendant, 100, true );
                    } else { //defendant win
                        //sub plaintiff credit value
                        SubCredit( plaintiff, 100 );
                    }
                }
            } else if ( c_arbit.type == ArbitType::AT_PRODUCT ) {
                //TODO:Report product
            } else if ( c_arbit.type == ArbitType::AT_ILLEGAL_INFO ) {
                //TODO:ILLEGAL INFO
            }
        } else {
            check( false, "Incomplete signature for updatearbit" );
        }
    }

    void bitsfleamain::SubCredit( const User& user, uint32_t value, bool cancel_reviewer )
    {
        _user_table.modify( user, same_payer, [&](auto& u){
            if( cancel_reviewer && u.is_reviewer ) {
                u.is_reviewer = false;
                if( u.credit_value >= _global.credit_reviewer_limit ) {
                    u.credit_value = _global.credit_reviewer_limit - value;
                } else {
                    u.credit_value -= value;
                }
                //cancel reviewer
                reviewer_index reviewer_table( _self, _self.value );
                auto rev_itr = reviewer_table.find( u.uid );
                if( rev_itr != reviewer_table.end() ) {
                    AddTableLog("reviewers"_n, OpType::OT_DELETE, rev_itr->uid);
                    reviewer_table.erase( rev_itr );
                }
                
            } else {
                u.credit_value -= value;
                if( u.credit_value < _global.credit_reviewer_limit && user.is_reviewer ) {
                    u.is_reviewer = false;
                }
            }
            if( u.credit_value < 1 ) {
                u.credit_value = 0;
                u.status = UserStatus::LOCK;
            }
        });
        AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
    }

    void bitsfleamain::SubCredit( uint64_t user_uid, uint32_t value, bool cancel_reviewer )
    {
        auto& user = _user_table.get( user_uid, "Invalid user uid" );
        SubCredit( user, value, cancel_reviewer );
    }

    void bitsfleamain::AddCredit( const User& user, uint32_t value )
    {
        _user_table.modify( user, same_payer, [&](auto& u){
            u.credit_value += value;
        });
        AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
    }
    void bitsfleamain::AddCredit( uint64_t user_uid, uint32_t value )
    {
        auto& user = _user_table.get( user_uid, "Invalid user uid" );
        AddCredit( user, value );
    }

    void bitsfleamain::RewardReferrer( const name& referrer )
    {
        if( _global.ref_sys_gift.amount > 0 && _global.ref_pool.amount >= _global.ref_sys_gift.amount ) {
            action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
                std::make_tuple( referrer, _global.ref_sys_gift, string("Reward referrer") )
            ).send();
            _global.ref_pool -= _global.ref_sys_gift;
        }
    }

    bool bitsfleamain::IsLockUser( uint64_t user_uid )
    {
        auto& user = _user_table.get( user_uid, "Invalid user id" );
        return IsLockUser( user );
    }
    bool bitsfleamain::IsLockUser( const User& user )
    {
        return user.status == UserStatus::LOCK;
    }
    
    void bitsfleamain::startsync()
    {
        if( _global.sync_data )
            return;
        require_auth( _self );
        uint32_t s = current_time_point().sec_since_epoch() - _global.dividend_last_time.sec_since_epoch();
        check( s >= _global.dividend_interval, "Dividend time is not up" );
        _global.sync_data = true;
        vector<asset> tmp;
        _global.last_income.swap( tmp );
        // get FMP supply
        tokenStats ts_table( _self, FMP.code().raw() );
        auto& fmp = ts_table.get( FMP.code().raw() );
        if( fmp.supply.amount > 0 ) {
            _global.dividend_last_supply = fmp.supply;
            int64_t one = 0;
            int64_t all = 0;
            for_each( _global.income.begin(), _global.income.end(), [&](auto& a){
                one = a.amount / fmp.supply.amount;
                if( one > 0 ) {
                    all = one * fmp.supply.amount;
                    a.amount -= all;
                    _global.last_income.push_back( asset( all, a.symbol ) );
                }
            });
        }
    }
    void bitsfleamain::endsync()
    {
        if( ! _global.sync_data )
            return;
        require_auth( _self );
        _global.sync_data = false;
        _global.dividend_last_time = time_point_sec(current_time_point().sec_since_epoch());
    }

    bool bitsfleamain::CheckSymbol( const symbol& symbol )
    {
        coin_index coin_table( _self, _self.value );
        auto itr = coin_table.find( symbol.code().raw() );
        bool flg = itr != coin_table.end();
        if( flg ) {
            flg = symbol.precision() == itr->sym.precision();
        }
        return flg;
    }

    void bitsfleamain::Withdraw( const name& user_eosid, const asset& quantity )
    {
        // require_auth( user_eosid );

        // check( quantity.is_valid(), "invalid quantity" );
        // check( CheckSymbol( quantity.symbol ), "Invalid symbol" );

        // auto balance = GetBalance( FLEA_PLATFORM, user_eosid, quantity.symbol );
        // check( balance.amount >= quantity.amount, "invalid quantity" );
        
    }

    void bitsfleamain::bindaddr( uint64_t uid, const name& user_eosid, const symbol& sym, const string& addr )
    {
        require_auth( user_eosid );

        check( CheckSymbol( sym ), "Invalid symbol" );
        check( sym != SYS && sym != FMP, "Invalid symbol" );
        check( addr.length() > 0 && addr.length() <= 100, "Invalid addr" );

        auto& user = _user_table.get( uid, "Invalid uid" );
        otheraddr_index oa_table( _self, _self.value );
        auto oa_idx = oa_table.get_index<"byuid"_n>();
        auto oa_itr = oa_idx.find( uid );
        bool is_exists = false;
        while( oa_itr != oa_idx.end() && oa_itr->uid == uid ) {
            if( oa_itr->coin_type == sym ) {
                is_exists = true;
                oa_idx.modify( oa_itr, same_payer, [&](auto& o){
                    o.addr = addr;
                });
                AddTableLog("otheraddr"_n, OpType::OT_UPDATE, oa_itr->id);
                break;
            }
            oa_itr++;
        }
        if( !is_exists ) {
            oa_table.emplace( _self, [&](auto& o){
                o.id = oa_table.available_primary_key();
                o.uid = uid;
                o.coin_type = sym;
                o.addr = addr;
                AddTableLog("otheraddr"_n, OpType::OT_INSERT, o.id);
            });
        }
    }

    void bitsfleamain::updatecoin( const symbol& sym, const asset& fee, bool is_out, const asset& max )
    {
        require_auth( _self );
        check( sym.is_valid(), "Invalid sym" );
        check( sym == fee.symbol, "Invalid sym" );
        coin_index coin_table( _self, _self.value );
        auto itr = coin_table.find( sym.code().raw() );
        if( itr == coin_table.end() ) { //add
            check( max.amount > 0, "max amount must be greater than 0");
            check( sym == max.symbol, "Invalid sym" );
            coin_table.emplace( _self, [&](auto& c){
                c.sym = sym;
                c.fee = fee;
                c.is_out = is_out;
            });
            create( _self, max, 0 );
        } else { //update
            coin_table.modify( itr, same_payer, [&](auto& c){
                c.fee = fee;
                c.is_out = is_out;
            });
        }
    }

    void bitsfleamain::setpayaddr( const uint128_t& order_id, const string& addr )
    {
        require_auth( _self );
        check( addr.length()>0 && addr.length()<=200, "Invalid addr" );
        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order_id" );
        order_table.modify( order, same_payer, [&](auto& o){
            o.pay_addr = addr;
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });
    }
    
}