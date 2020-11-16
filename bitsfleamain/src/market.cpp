#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"
#include "../../tools/include/tool.hpp"

using namespace eosio;

namespace rareteam {

    void bitsfleamain::publish( uint64_t uid, const Product& product, const optional<ProductAuction>& pa )
    {
        require_auth( _self );
        auto& user = _user_table.get( uid, "Invalid account uid" );
        require_auth( user.eosid );
        check( IsLockUser( user ) == false, "Account is locked." );
        check( product.price.symbol == product.postage.symbol, "Inconsistent payment methods." );
        check( CheckSymbol( product.price.symbol ), "This currency is not currently supported." );

        Category_index category_table( _self, _self.value);
        auto cat_itr = category_table.find( product.category );
        check( cat_itr != category_table.end(), "Invalid product category" );

        product_index pro_table( _self, _self.value );
        auction_index pa_table( _self, _self.value );

        if( product.pid > 0 ){
            auto& o_product = pro_table.get( product.pid, "Product does not exist.");
            check( o_product.status == ProductStatus::DELISTED, "Only modify items that have been delisted.");
            pro_table.modify( o_product, same_payer, [&]( auto& p){
                p.title = product.title;
                p.description = product.description;
                p.photos = product.photos;
                p.category = product.category;
                p.status = ProductStatus::PUBLISH;
                p.is_new = product.is_new;
                p.is_returns = product.is_returns;
                p.sale_method = product.sale_method;
                p.price = product.price;
                p.transaction_method = product.transaction_method;
                p.postage = product.postage;
                p.position = product.position;
                p.release_time = time_point_sec(current_time_point().sec_since_epoch());
                AddTableLog("products"_n, OpType::OT_UPDATE, p.pid );
            });
            if( product.sale_method == 1 ){
                check( pa.has_value(), "Please provide auction information." );
                auto& tpa = pa.value();
                auto auction_pid_index = pa_table.get_index<"bypid"_n>();
                auto api_itr = auction_pid_index.find( product.pid );
                if( api_itr ==  auction_pid_index.end() ){
                    auto tpa = pa.value();
                    pa_table.emplace( _self, [&]( auto& pa_item ) {
                        pa_item.id = pa_table.available_primary_key();
                        pa_item.id = pa_item.id == 0 ? 1 : pa_item.id;
                        pa_item.pid = product.pid;
                        pa_item.security = tpa.security;
                        pa_item.markup = tpa.markup;
                        pa_item.current_price = product.price;
                        pa_item.auction_times = 0;
                        pa_item.last_price_user = 0;
                        pa_item.start_time = tpa.start_time;
                        pa_item.end_time = tpa.end_time;
                        AddTableLog( "proauction"_n, OpType::OT_INSERT, pa_item.id );
                    });
                } else {
                    auction_pid_index.modify( api_itr, same_payer, [&]( auto& pa_item ){
                        pa_item.security = tpa.security;
                        pa_item.markup = tpa.markup;
                        pa_item.current_price = product.price;
                        pa_item.auction_times = 0;
                        pa_item.last_price_user = 0;
                        pa_item.start_time = tpa.start_time;
                        pa_item.end_time = tpa.end_time;
                        AddTableLog( "proauction"_n, OpType::OT_UPDATE, pa_item.id );
                    });
                }
            }
            return;
        }
        auto pro_itr = pro_table.emplace( _self, [&]( auto& p ) {
            p.pid = pro_table.available_primary_key();
            p.pid = p.pid == 0 ? 1 : p.pid;
            p.uid = uid;
            p.title = product.title;
            p.description = product.description;
            p.photos = product.photos;
            p.category = product.category;
            p.status = ProductStatus::PUBLISH;
            p.is_new = product.is_new;
            p.is_returns = product.is_returns;
            p.sale_method = product.sale_method;
            p.price = product.price;
            p.transaction_method = product.transaction_method;
            p.postage = product.postage;
            p.position = product.position;
            p.release_time = time_point_sec(current_time_point().sec_since_epoch());
            AddTableLog("products"_n, OpType::OT_INSERT, p.pid );
        });
        if( pro_itr->sale_method == 1 ) {
            check( pa.has_value(), "Please provide auction information" );
            auto tpa = pa.value();
            pa_table.emplace( _self, [&]( auto& pa_item ) {
                pa_item.id = pa_table.available_primary_key();
                pa_item.id = pa_item.id == 0 ? 1 : pa_item.id;
                pa_item.pid = pro_itr->pid;
                pa_item.security = tpa.security;
                pa_item.markup = tpa.markup;
                pa_item.current_price = product.price;
                pa_item.auction_times = 0;
                pa_item.last_price_user = 0;
                pa_item.start_time = tpa.start_time;
                pa_item.end_time = tpa.end_time;
                AddTableLog( "proauction"_n, OpType::OT_INSERT, pa_item.id );
            });
        }
        _user_table.modify( user, same_payer, [&](auto& u){
            u.posts_total += 1;
        });
        AddTableLog("users"_n, OpType::OT_UPDATE, user.uid);
    }

    void bitsfleamain::pulloff( uint64_t seller_uid, const name& seller_eosid, uint32_t pid )
    {
        require_auth( seller_eosid );

        check( is_account( seller_eosid), "Invalid seller eosid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product id" );
        check( product.status == ProductStatus::NORMAL, "The product is not normal status" );
        check( product.uid == seller_uid, "This product does not belong to you" );

        pro_table.modify( product, same_payer, [&](auto& p){
            p.status = ProductStatus::DELISTED;
        });
        AddTableLog("products"_n, OpType::OT_UPDATE, product.pid );
    }

    void bitsfleamain::review( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t pid, bool is_delisted, optional<string>& memo )
    {
        require_auth( reviewer_eosid );
        check( IsLockUser( reviewer_uid ) == false, "Account is locked" );
        if( memo.has_value() ) {
            check( memo.value().length()>0 && memo.value().length()<=1000, "Invalid memo" );
        }
        if( is_delisted ) {
            check( memo.has_value(), "must provide reasons for delisting" );
        }

        reviewer_index re_table( _self, _self.value );
        auto re_uid_index = re_table.get_index<"byuid"_n>();
        auto re_itr = re_uid_index.find( reviewer_uid );
        product_index pro_table( _self, _self.value );
        auto pro_itr = pro_table.find( pid );
        if( re_itr != re_uid_index.end() && pro_itr != pro_table.end() && pro_itr->status == ProductStatus::PUBLISH ) {
            check( reviewer_uid != pro_itr->uid, "can’t review the products posted by myself" );
            proaudit_index audit_table( _self, _self.value );
            auto audit_pid_index = audit_table.get_index<"bypid"_n>();
            auto old_a_itr = audit_pid_index.find( pid );
            bool is_reward = old_a_itr == audit_pid_index.end();
            audit_table.emplace( _self, [&]( auto& a ){
                a.id = audit_table.available_primary_key();
                a.id = a.id == 0 ? 1 : a.id;
                a.pid = pid;
                a.reviewer_uid = reviewer_uid;
                a.is_delisted = is_delisted;
                a.review_details = memo.has_value() ? memo.value() : string("");
                a.review_time = time_point_sec(current_time_point().sec_since_epoch());
                AddTableLog( "proaudits"_n, OpType::OT_INSERT, a.pid );
            });

            pro_table.modify( pro_itr, same_payer, [&]( auto& p ){
                if( is_delisted ){
                    p.status = ProductStatus::DELISTED;
                    SubCredit( pro_itr->uid, 5 );//delisted sub 5 credit 
                } else {
                    p.status = ProductStatus::NORMAL;
                    if( is_reward )
                        AddCredit( pro_itr->uid, 1 );//not delisted  add 1 credit
                }
                p.reviewer = reviewer_uid;
                AddTableLog("products"_n, OpType::OT_UPDATE, p.pid );
            });
            // point logic
            if( !is_delisted && is_reward ){
                // reward publisher
                auto& publisher = _user_table.get( pro_itr->uid, "Invalid uid for review");
                if( _global.gift_publish_product.amount > 0 && _global.transaction_pool.amount >= _global.gift_publish_product.amount ) {
                    action(permission_level{_self, "active"_n}, _self, "issue"_n,
                        std::make_tuple( publisher.eosid, _global.gift_publish_product, string("Reward publish product") )
                    ).send();
                    _global.transaction_pool -= _global.gift_publish_product;
                }
            }
            // reviewer salary
            if( _global.salary_pool.amount >= _global.review_salary_product.amount ) {
                action(permission_level{_self, "active"_n}, _self, "issue"_n,
                    std::make_tuple( reviewer_eosid, _global.review_salary_product, string("publish product salary") )
                ).send();
                _global.salary_pool -= _global.review_salary_product;
            }
        }
    }

    void bitsfleamain::BidAuction( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const asset& price )
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
        auto& user = _user_table.get( buyer_uid, "Invalid account buyer_uid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product pid" );
        check( product.status == ProductStatus::NORMAL, "This product cannot be traded" );
        check( product.price.symbol == price.symbol, "Invalid asset symbol" );

        auction_index auction_table( _self, _self.value );
        auto pid_index = auction_table.get_index<"bypid"_n>();
        auto& auction = pid_index.get( uint64_t(pid), "Invalid auction product" );
        auto now = time_point_sec(current_time_point().sec_since_epoch());
        check( auction.start_time <= now, "Auction has not yet begin" );
        check( auction.end_time > now, "The auction has ended" );
        auto new_price = auction.current_price + auction.markup;
        check( price.amount >= new_price.amount, "Bid is too low" );

        pid_index.modify( pid_index.iterator_to(auction), same_payer, [&](auto& a){
            a.current_price = price;
            a.auction_times += 1;
            a.last_price_user = buyer_uid;
            AddTableLog( "proauction"_n, OpType::OT_UPDATE, a.pid );
        });
    }

    void bitsfleamain::cancelorder( uint64_t buyer_uid, const name& buyer_eosid, const uint128_t& order_id)
    {
        require_auth( buyer_eosid );

        order_index order_table( _self, _self.value );
        auto order_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_index.find( order_id );
        check( order_itr != order_index.end(), "Invalid order id" );
        check( (order_itr->status == OrderStatus::OS_PENDING_PAYMENT || order_itr->status == OrderStatus::OS_CANCELLED), "Invalid order status");
        check( order_itr->buyer_uid == buyer_uid, "The order does not belong to you" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( order_itr->pid, "Invalid product pid" );
        pro_table.modify( product, same_payer, [&](auto& p){
            p.status = ProductStatus::NORMAL;
        });
        AddTableLog("products"_n, OpType::OT_UPDATE, product.pid );
        AddTableLog( "orders"_n, OpType::OT_DELETE, order_itr->id );
        order_index.erase( order_itr );
    }

    void bitsfleamain::placeorder( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const uint128_t& order_id, uint32_t to_addr, optional<string>& pay_addr)
    {
        require_auth( buyer_eosid );
        if( order_id > 0 ) {
            require_auth( _self );
        }
        // check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
        auto& user = _user_table.get( buyer_uid, "Invalid account buyer_uid" );
        check( IsLockUser( user ) == false, "Account is locked" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product pid" );
        check( product.status == ProductStatus::NORMAL, "This product cannot be traded" );

        uint128_t oid = 0;
        auto ct = current_time_point().sec_since_epoch();
        if( order_id == 0 ) {
            oid = ((uint128_t(buyer_uid) << 64) | (uint128_t(pid) << 32)) | ct;
        } else {
            if( get_buyer_uid_by_orderid(order_id) == buyer_uid && get_pid_by_orderid(order_id) == pid ) {
                oid = order_id;
            } else {
                oid = ((uint128_t(buyer_uid) << 64) | (uint128_t(pid) << 32)) | ct;
            }
        }
        order_index order_table( _self, _self.value );
        auto order_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_index.find( oid );
        check( order_itr == order_index.end(), "Order number already exists" );
        //create order
        //fixed price
        order_table.emplace( _self, [&](auto& o){
            o.id = order_table.available_primary_key();
            if( o.id == 0 ) o.id += 1;
            o.oid = oid;
            o.pid = pid;
            o.seller_uid = product.uid;
            o.buyer_uid = buyer_uid;
            o.price = product.price;
            o.postage = product.postage;
            o.status = OrderStatus::OS_PENDING_PAYMENT;
            o.to_addr = to_addr;
            if( pay_addr.has_value() ) {
                o.pay_addr = pay_addr.value();
            }
            o.create_time = time_point_sec(ct);
            o.pay_time_out = time_point_sec(ct + _global.pay_time_out);
            AddTableLog( "orders"_n, OpType::OT_INSERT, o.id );
        });
        //update product status
        pro_table.modify( product, same_payer, [&](auto& p){
            p.status = ProductStatus::LOCKED;
        });
        AddTableLog("products"_n, OpType::OT_UPDATE, product.pid );
    }

    void bitsfleamain::PayOrder( uint128_t order_id, const asset& quantity )
    {
        order_index order_table( _self, _self.value );
        auto order_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_index.find( order_id );
        check( order_itr != order_index.end(), "PayOrder:Invalid order id" );
        auto& order = (*order_itr);
        check( order.price.symbol == quantity.symbol, "Invalid order symbol" );
        check( quantity.amount == (order.postage + order.price).amount, "Invalid order amount" );
        check( order.status == OrderStatus::OS_PENDING_PAYMENT, "Invalid order status");

        time_point_sec pay_time = time_point_sec(current_time_point().sec_since_epoch());
        check( pay_time < order.pay_time_out, "Order has expired");
        if( pay_time < order.pay_time_out ) {
            order_index.modify( order_itr, same_payer, [&](auto& o){
                o.status = OrderStatus::OS_PENDING_SHIPMENT;
                o.pay_time = pay_time;
                o.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
                AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
            });
        } else {
            //update product status
            product_index pro_table( _self, _self.value );
            auto& product = pro_table.get( order.pid, "Invalid product pid" );
            pro_table.modify( product, same_payer, [&](auto& p){
                p.status = ProductStatus::NORMAL;
            });
            AddTableLog("products"_n, OpType::OT_UPDATE, product.pid );
            AddTableLog( "orders"_n, OpType::OT_DELETE, order.id );
            order_table.erase( order );
            // check( false, "Order has expired");
        }
    }

    void bitsfleamain::OnEOSTransfer( const name& from, const name& to, const asset& quantity, const string& memo )
    {
        if( to != _self ) return;
        require_auth( from );
        if( quantity.symbol != SYS ) return;
        check( quantity.amount > 0, "Invalid quantity" );
        bool is_payorder = memo.find( string("p:") ) == 0;
        if( !is_payorder ) return;

        auto info = split( memo, ":" );
        uint128_t order_id = get_orderid( info[1] );

        PayOrder( order_id, quantity );
    }

    void bitsfleamain::OnIBCTransfer( const name& from, const name& to, const asset& quantity, const string& memo )
    {
        if( to != _self ) return;
        check( quantity.amount > 0, "Invalid quantity" );
        if( CheckSymbol( quantity.symbol ) == false ) return;
        bool is_payorder = memo.find( "p:" ) == 0;

        if( is_payorder ) {
            auto info = split( memo, ":" );
            uint128_t order_id = get_orderid( info[1] );
            PayOrder( order_id, quantity );
        }
    }

    void bitsfleamain::OnMyTransfer( const name& from, const name& to, const asset& quantity, const string& memo )
    {
        if( to != _self ) return;
        check( quantity.amount > 0, "Invalid quantity" );
        if( CheckSymbol( quantity.symbol ) == false ) return;
        bool is_payorder = memo.find( "p:" ) == 0;
        bool is_withdraw = memo.find( "w:" ) == 0;
        if( !is_payorder && !is_withdraw ) return;

        auto info = split( memo, ":" );
        if( is_payorder ) {
            uint128_t order_id = get_orderid( info[1] );
            PayOrder( order_id, quantity );
        } else if( is_withdraw ) {
            auto payer = from;
            if( info.size() == 2 && info[1].length() > 0 ){
                payer = name(info[1]);
            }
            check( quantity.symbol != SYS && quantity.symbol != FMP, "Invalid quantity symbol" );
            coin_index coin_table( _self, _self.value );
            auto& coin = coin_table.get( quantity.symbol.code().raw() );
            check( coin.is_out, "Withdrawal closed" );
            check( quantity.amount > coin.fee.amount, "The withdraw amount is too small" );
            string addr = "";
            auto user_idx = _user_table.get_index<"eosid"_n>();
            auto user_itr = user_idx.find( payer.value );
            check( user_itr != user_idx.end(), "This account is not a platform user" );

            otheraddr_index oa_table( _self, _self.value );
            auto oa_idx = oa_table.get_index<"byuid"_n>();
            auto oa_itr = oa_idx.find( user_itr->uid );
            while( oa_itr != oa_idx.end() ) {
                if( oa_itr->coin_type == quantity.symbol ) {
                    addr = oa_itr->addr;
                    break;
                }
                oa_itr++;
            }
            check( addr.length() > 0, "Withdraw address not yet bound" );

            othersettle_index os_table( _self, _self.value );
            os_table.emplace( _self, [&](auto& os){
                os.id = os_table.available_primary_key();
                os.id = os.id == 0 ? 1 : os.id;
                os.uid = user_itr->uid;
                os.order_id = 0;
                os.amount = quantity - coin.fee; //Withdrawal fees have been deducted
                os.status = OtherSettleStatus::OSS_NORMAL;
                os.addr = addr;
                os.memo = "withdraw coin";
                os.start_time = time_point_sec(current_time_point().sec_since_epoch());
            });
            // sub_balance( _self, coin.fee );
            // add_balance( _global.gateway, coin.fee, _self );
            sub_balance( _self, quantity );
            add_balance( _global.gateway, quantity, _self );
        }
    }

    void bitsfleamain::reshipment( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& number )
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid buyer eosid" );
        check( number.length() <= 50, "number too long" );

        proreturn_index repro_table( _self, _self.value );
        auto repro_order_index = repro_table.get_index<"byorderid"_n>();
        auto& repro = repro_order_index.get( order_id, "invalid order id" );
        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        check( buyer_uid == order_itr->buyer_uid, "This order does not belong to you" );
        check( repro.status == ReturnStatus::RS_PENDING_SHIPMENT, "This order is not ready for shipment" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        repro_order_index.modify( repro_order_index.iterator_to(repro), same_payer, [&](auto& re){
            re.shipment_number = number;
            re.ship_time = current_time;
            re.status = ReturnStatus::RS_PENDING_RECEIPT;
            re.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            AddTableLog( "returns"_n, OpType::OT_UPDATE, re.id );
        });

        // shipment delivery timeout
        // if( current_time > order.ship_time_out ) {
        //     SubCredit( buyer_uid, 5 );
        // }
    }

    void bitsfleamain::shipment( uint64_t seller_uid, const name& seller_eosid, uint128_t order_id, const string& number)
    {
        require_auth( seller_eosid );
        check( number.length() <= 50, "number too long" );

        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        check( seller_uid == order.seller_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_SHIPMENT, "This order is not ready for shipment" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_oid_index.modify( order_itr, same_payer, [&](auto& o){
            o.shipment_number = number;
            o.ship_time = current_time;
            o.status = OrderStatus::OS_PENDING_RECEIPT;
            o.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });

        // shipment delivery timeout
        // if( current_time > order.ship_time_out ) {
        //     SubCredit( seller_uid, 5 );
        // }
    }

    void bitsfleamain::PayCoin( const string& stroid, const Order& order, const User& seller, const User& buyer, const asset& seller_income, const asset& devops_income, const asset& referrer_income, const name& contract)
    {
        transaction trx;
        string memo = string("complete order ") + stroid;
        action a1 = action( permission_level{_self, ACTIVE_PERMISSION}, contract, ACTION_NAME_TRANSFER,
            std::make_tuple( _self, seller.eosid, seller_income, memo )
        );
        trx.actions.emplace_back( a1 );
        if( devops_income.amount > 0 ) {
            action a2 = action( permission_level{_self, ACTIVE_PERMISSION}, contract, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, _global.devops, devops_income, memo )
            );
            trx.actions.emplace_back( a2 );
        }
        if( buyer.referrer > 0 && referrer_income.amount > 0 ){
            auto& referrer = _user_table.get( buyer.referrer, "Invalid order referrer uid" );
            action a3 = action ( permission_level{_self, ACTIVE_PERMISSION}, contract, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, referrer.eosid, referrer_income, "Referral commission " + stroid )
            );
            trx.actions.emplace_back( a3 );
        }
        trx.delay_sec = 5;
        trx.send( (uint128_t(("settle"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
    }

    void bitsfleamain::Settle( const Order& order, const User& seller, const User& buyer )
    {
        string stroid = uint128ToString( order.id );
        auto total = order.price + order.postage;
        auto income = asset( int64_t(double(total.amount) * _global.fee_ratio), total.symbol );
        auto devops_income = asset( int64_t(double(income.amount) * _global.devops_rate), total.symbol );
        auto amount = total - income;
        if( devops_income.amount > 0 )
            income -= devops_income;
        auto comm = asset( int64_t(double(income.amount) * _global.ref_commission_rate), income.symbol );
        if( buyer.referrer > 0 && comm.amount > 0 && IsLockUser( buyer.referrer ) == false ){
            income -= comm;
        } else {
            comm = asset( 0, income.symbol );
        }
        if( order.price.symbol == SYS ) {
            PayCoin( stroid, order, seller, buyer, amount, devops_income, comm, NAME_EOSIO_TOKEN );
        } else {
            //other blockchain
            otheraddr_index oa_table( _self, _self.value );
            auto uid_idx = oa_table.get_index<"byuid"_n>();
            bool is_bind = false;
            string addr = "";
            auto oa_itr = uid_idx.find( order.seller_uid );
            while ( oa_itr != uid_idx.end() && oa_itr->uid == order.seller_uid )
            {
                if( order.price.symbol == oa_itr->coin_type && oa_itr->addr.length() != 0 ) {
                    is_bind = true;
                    addr = oa_itr->addr;
                    break;
                }
                oa_itr++;
            }
            if( is_bind ) { // create settle log
                string memo = "complete order " + stroid;
                othersettle_index os_table( _self, _self.value );
                os_table.emplace( _self, [&](auto& os){
                    os.id = os_table.available_primary_key();
                    os.id = os.id == 0 ? 1 : os.id;
                    os.uid = order.seller_uid;
                    os.order_id = order.id;
                    os.amount = amount;
                    os.status = OtherSettleStatus::OSS_NORMAL;
                    os.addr = addr;
                    os.memo = memo;
                    os.start_time = time_point_sec(current_time_point().sec_since_epoch());
                });
                transaction trx;
                if( devops_income.amount > 0 ) {
                    action a1 = action( permission_level{_self, ACTIVE_PERMISSION}, _self, ACTION_NAME_TRANSFER,
                        std::make_tuple( _self, _global.devops, devops_income, memo )
                    );
                    trx.actions.emplace_back( a1 );
                }
                if( buyer.referrer > 0 && comm.amount > 0 ){
                    auto& referrer = _user_table.get( buyer.referrer, "Invalid order referrer uid" );
                    action a2 = action( permission_level{_self, ACTIVE_PERMISSION}, _self, ACTION_NAME_TRANSFER,
                        std::make_tuple( _self, referrer.eosid, comm, "Referral commission" + stroid )
                    );
                    trx.actions.emplace_back( a2 );
                }
                if( trx.actions.size() > 0 ) {
                    trx.delay_sec = 5;
                    trx.send( (uint128_t(("settle"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
                }
            } else { // bitsfleamain transfer
                PayCoin( stroid, order, seller, buyer, amount, devops_income, comm, _self );
            }
        }

        auto itr = find_if( _global.income.begin(), _global.income.end(), [&](asset& a){
            return a.symbol == total.symbol;
        });
        if( itr != _global.income.end() ) {
            asset sys = (*itr) + income;
            _global.income.erase( itr );
            _global.income.push_back( sys );
        } else {
            _global.income.push_back( income );
        }
    }

    void bitsfleamain::EndOrder( const Order& order )
    {
        auto& seller = _user_table.get( order.seller_uid, "Invalid order seller_uid" );
        auto& buyer = _user_table.get( order.buyer_uid, "Invalid order buyer_uid" );
        _user_table.modify( seller, same_payer, [&](auto& u){
            u.sell_total += 1;
        });
        _user_table.modify( buyer, same_payer, [&](auto& u){
            u.buy_total += 1;
        });
        AddTableLog("users"_n, OpType::OT_UPDATE, seller.uid);
        AddTableLog("users"_n, OpType::OT_UPDATE, buyer.uid);
        Settle( order, seller, buyer );
    }

    void bitsfleamain::closesettle( uint64_t os_id, const string& trx_id )
    {
        require_auth( _self );

        check( trx_id.length() <= 100, "trx_id is too long" );
        othersettle_index os_table( _self, _self.value );
        auto& os = os_table.get( os_id, "Invalid other settle id" );

        os_table.modify( os, same_payer, [&](auto& o){
            o.end_time = time_point_sec(current_time_point().sec_since_epoch());
            o.status = OtherSettleStatus::OSS_PAID;
            o.trx_id = trx_id;
        });
        action( permission_level{_self, ACTIVE_PERMISSION}, _self, ACTION_NAME_TRANSFER,
            std::make_tuple( _self, _global.gateway, os.amount, os.memo )
        ).send();
        uint32_t count = get_size( os_table );
        if( count > 500 ) {
            auto itr = os_table.begin();
            if( itr != os_table.end() && itr->status == OtherSettleStatus::OSS_PAID )
                os_table.erase( itr );
        }
    }

    void bitsfleamain::Refund( const Order& order )
    {
        auto& buyer = _user_table.get( order.buyer_uid, "Invalid order seller_uid" );
        auto& seller = _user_table.get( order.seller_uid, "Invalid order seller_uid" );

        string memo = string("returns order ") + uint128ToString( order.id );
        transaction trx;
        if( order.price.symbol == SYS ) { //EOS
            action a1 = action( permission_level{_self, ACTIVE_PERMISSION}, NAME_EOSIO_TOKEN, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, buyer.eosid, order.price, memo )
            );
            action a2 =action( permission_level{_self, ACTIVE_PERMISSION}, NAME_EOSIO_TOKEN, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, seller.eosid, order.postage, memo )
            );
            trx.actions.emplace_back( a1 );
            trx.actions.emplace_back( a2 );
        } else {
            action a3 = action( permission_level{_self, ACTIVE_PERMISSION}, _self, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, buyer.eosid, order.price, memo )
            );
            action a4 = action( permission_level{_self, ACTIVE_PERMISSION}, _self, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, seller.eosid, order.postage, memo )
            );
            trx.actions.emplace_back( a3 );
            trx.actions.emplace_back( a4 );
        }
        trx.delay_sec = 5;
        trx.send( (uint128_t(("refund"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
    }

    void bitsfleamain::conreceipt( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id )
    {
        require_auth( buyer_eosid );

        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        check( buyer_uid == order.buyer_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_RECEIPT, "The order status is not OS_PENDING_RECEIPT" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_oid_index.modify( order_itr, same_payer, [&](auto& o){
            o.receipt_time = current_time;
            o.status = OrderStatus::OS_COMPLETED;
            o.end_time = current_time;
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });
        product_index product_table( _self, _self.value );
        auto pro_itr = product_table.find( order.pid );
        if( pro_itr != product_table.end() ) {
            product_table.modify( pro_itr, same_payer, [&](auto& p){
                p.status = ProductStatus::COMPLETED; 
            });
            AddTableLog("products"_n, OpType::OT_UPDATE, pro_itr->pid );
        }
        EndOrder( order );

        // credit
        AddCredit( order.seller_uid, 5 );
        if( current_time > order.receipt_time_out ) {
            SubCredit( buyer_uid, 5 );
        } else {
            AddCredit( buyer_uid, 5 );
        }
        // points
        int64_t val = int64_t(double(order.price.amount) * _global.transaction_gift_rate);
        if( val > 1 && val <= (_global.transaction_pool.amount * 2) ) {
            auto& seller = _user_table.get( order.seller_uid, "Invalid seller uid for conreceipt" );
            asset point = asset( val, FMP );
            transaction trx;
            action a1 = action(permission_level{_self, "active"_n}, _self, "issue"_n,
                std::make_tuple( buyer_eosid, point, string("transaction gift") )
            );
            action a2 = action(permission_level{_self, "active"_n}, _self, "issue"_n,
                std::make_tuple( seller.eosid, point, string("transaction gift") )
            );
            trx.actions.emplace_back( a1 );
            trx.actions.emplace_back( a2 );
            trx.delay_sec = 5;
            trx.send( (uint128_t(("conreceipt"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
        }
    }

    void bitsfleamain::reconreceipt( uint64_t seller_uid, const name& seller_eosid, uint128_t order_id )
    {
        require_auth( seller_eosid );

        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        check( seller_uid == order.seller_uid, "This order does not belong to you" );

        proreturn_index repro_table( _self, _self.value );
        auto repro_order_index = repro_table.get_index<"byorderid"_n>();
        auto& repro = repro_order_index.get( order_id, "proReturn invalid order id" );
        check( repro.status == ReturnStatus::RS_PENDING_RECEIPT, "The order status is not RS_PENDING_RECEIPT" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        repro_order_index.modify( repro_order_index.iterator_to(repro), same_payer, [&](auto& re){
            re.receipt_time = current_time;
            re.status = ReturnStatus::RS_COMPLETED;
            re.end_time = current_time;
            AddTableLog( "returns"_n, OpType::OT_UPDATE, re.id );
        });
        order_oid_index.modify( order_itr, same_payer, [&](auto& o){
            o.status = OrderStatus::OS_CANCELLED;
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });
        product_index product_table( _self, _self.value );
        auto pro_itr = product_table.find( order.pid );
        if( pro_itr != product_table.end() ) {
            product_table.modify( pro_itr, same_payer, [&](auto& p){
                p.status = ProductStatus::NORMAL; 
            });
            AddTableLog("products"_n, OpType::OT_UPDATE, pro_itr->pid );
        }
        //Refund
        Refund( order );
        // receipt delivery timeout
        if( current_time > repro.receipt_time_out ) {
            SubCredit( seller_uid, 5 );
        }
    }

    void bitsfleamain::deferreceipt( uint64_t user_uid, const name& user_eosid, uint128_t order_id )
    {
        require_auth( user_eosid );

        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        check( order.buyer_uid == user_uid, "This order does not belong to you" );
        check( order.delayed_count < _global.max_deferr_times, "Has been postponed three times" );

        order_oid_index.modify( order_itr, same_payer, [&](auto& o){
            o.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            o.delayed_count += 1;
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });
    }

    void bitsfleamain::deferreturn( uint64_t user_uid, const name& user_eosid, uint128_t order_id )
    {
        require_auth( user_eosid );

        order_index order_table( _self, _self.value );
        proreturn_index proreturn_table( _self, _self.value );

        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        
        auto proreturn_order_index = proreturn_table.get_index<"byorderid"_n>();
        auto& proreturn = proreturn_order_index.get( order_id, "Invalid order id for returns table" );
        check( order.seller_uid == user_uid, "This order does not belong to you" );
        check( proreturn.delayed_count < _global.max_deferr_times, "Has been postponed three times" );

        proreturn_order_index.modify( proreturn_order_index.iterator_to(proreturn), same_payer, [&](auto& pr){
            pr.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            pr.delayed_count += 1;
            AddTableLog( "returns"_n, OpType::OT_UPDATE, pr.id );
        });

    }

    void bitsfleamain::returns( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& reasons, uint32_t to_addr )
    {
        require_auth( buyer_eosid );

        check( is_account( buyer_eosid ), "Invalid buyer eosid" );
        check( reasons.length() <= 300, "reasons too long" );

        order_index order_table( _self, _self.value );
        auto order_oid_index = order_table.get_index<"byoid"_n>();
        auto order_itr = order_oid_index.find( order_id );
        check( order_itr != order_oid_index.end(), "Invalid order id" );
        auto& order = *order_itr;
        check( order.buyer_uid == buyer_uid, "Invalid buyer uid" );
        check( order.status == OrderStatus::OS_PENDING_RECEIPT, "product already received can be returned" );

        //is_returns
        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( order.pid, "Invalid product id" );
        check( product.is_returns, "This item does not support returns" );

        order_oid_index.modify( order_itr, same_payer, [&](auto& o){
            o.status = OrderStatus::OS_RETURN;
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
        });

        proreturn_index res_table( _self, _self.value );
        res_table.emplace( _self, [&](auto& r){
            r.id = res_table.available_primary_key();
            r.id = r.id == 0 ? 1 : r.id;
            r.order_id = order_id;
            r.pid = order.pid;
            r.order_price = order.price;
            r.status = ReturnStatus::RS_PENDING_SHIPMENT;
            r.reasons = reasons;
            r.to_addr = to_addr;
            r.create_time = time_point_sec(current_time_point().sec_since_epoch());
            r.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
            AddTableLog( "returns"_n, OpType::OT_INSERT, r.id );
        });

    }
    

    
}