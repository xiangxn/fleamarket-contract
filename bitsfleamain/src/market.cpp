#include <eosio/eosio.hpp>
#include <eosio/time.hpp>

#include "include/bitsfleamain.hpp"
#include "../../tools/include/tool.hpp"

using namespace eosio;

namespace rareteam {

    void bitsfleamain::publish( uint64_t uid, const Product& product, const ProductAuction& pa )
    {
        require_auth( _self );
        auto& user = _user_table.get( uid, "Invalid account uid" );

        product_index pro_table( _self, _self.value );
        auto pro_itr = pro_table.emplace( _self, [&]( auto& p ) {
            p.pid = pro_table.available_primary_key();
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
        });
        if( pa.id > 0 ) {
            auction_index pa_table( _self, _self.value );
            pa_table.emplace( _self, [&]( auto& pa_item ) {
                pa_item.id = pa_table.available_primary_key();
                pa_item.pid = pro_itr->pid;
                pa_item.security = pa.security;
                pa_item.markup = pa.markup;
                pa_item.current_price = product.price;
                pa_item.auction_times = 0;
                pa_item.last_price_user = 0;
                pa_item.start_time = pa.start_time;
                pa_item.end_time = pa.end_time;
            });
        }
    }

    void bitsfleamain::pulloff( uint64_t seeler_uid, const name& seller_eosid, uint32_t pid )
    {
        require_auth( seller_eosid );

        check( is_account( seller_eosid), "Invalid seller eosid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product id" );
        check( product.status == ProductStatus::NORMAL, "The product is not normal status" );
        check( product.uid == seeler_uid, "This product does not belong to you" );

        pro_table.modify( product, same_payer, [&](auto& p){
            p.status = ProductStatus::DELISTED;
        });
    }

    void bitsfleamain::review( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t pid, bool is_delisted, string& memo )
    {
        require_auth( reviewer_eosid );

        reviewer_index re_table( _self, _self.value );
        auto re_itr = re_table.find( reviewer_uid );
        product_index pro_table( _self, _self.value );
        auto pro_itr = pro_table.find( pid );
        if( re_itr != re_table.end() && pro_itr != pro_table.end() && pro_itr->status == ProductStatus::PUBLISH ) {
            proaudit_index audit_table( _self, _self.value );
            audit_table.emplace( _self, [&]( auto& a ){
                a.id = audit_table.available_primary_key();
                a.pid = pid;
                a.reviewer_uid = reviewer_uid;
                a.is_delisted = is_delisted;
                a.review_details = memo;
                a.review_time = time_point_sec(current_time_point().sec_since_epoch());
            });

            pro_table.modify( pro_itr, same_payer, [&]( auto& p ){
                if( is_delisted ){
                    p.status = ProductStatus::DELISTED;
                    SubCredit( pro_itr->uid, 5 );//delisted sub 5 credit 
                } else {
                    p.status = ProductStatus::NORMAL;
                }
                p.reviewer = reviewer_uid;
            });
            // point logic
            if( !is_delisted ){
                // reward publisher
                auto& publisher = _user_table.get( pro_itr->uid, "Invalid uid for review");
                if( _global.gift_publish_product.amount > 0 && _global.transaction_pool.amount >= _global.gift_publish_product.amount ) {
                    action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
                        std::make_tuple( publisher.eosid, _global.gift_publish_product, string("Reward publish product") )
                    ).send();
                    _global.transaction_pool -= _global.gift_publish_product;
                }
                _user_table.modify( publisher, same_payer, [&](auto& u){
                    u.posts_total += 1;
                });
            }
            // reviewer salary
            if( _global.salary_pool.amount >= _global.review_salary_product.amount ) {
                action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
                    std::make_tuple( reviewer_eosid, _global.review_salary_product, string("publish product salary") )
                ).send();
                _global.salary_pool -= _global.review_salary_product;
            }
        }
    }

    void bitsfleamain::bidauction( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const asset& price )
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
        auto& user = _user_table.get( buyer_uid, "Invalid account buyer_uid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product pid" );
        check( product.status == ProductStatus::NORMAL, "This product cannot be traded" );
        check( product.price.symbol == price.symbol, "Invalid asset symbol" );

        auction_index auction_table( _self, _self.value );
        auto& auction = auction_table.get( pid, "Invalid auction product" );
        auto now = time_point_sec(current_time_point().sec_since_epoch());
        check( auction.start_time <= now, "Auction has not yet begin" );
        check( auction.end_time > now, "The auction has ended" );
        auto new_price = auction.current_price + auction.markup;
        check( price.amount >= new_price.amount, "Bid is too low" );

        auction_table.modify( auction, same_payer, [&](auto& a){
            a.current_price = price;
            a.auction_times += 1;
            a.last_price_user = buyer_uid;
        });
    }

    void bitsfleamain::placeorder( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid)
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
        auto& user = _user_table.get( buyer_uid, "Invalid account buyer_uid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product pid" );
        check( product.status == ProductStatus::NORMAL, "This product cannot be traded" );

        //create order
        //fixed price
        order_index order_table( _self, _self.value );
        order_table.emplace( _self, [&](auto& o){
            o.id = ((uint128_t(buyer_uid) << 64) | (uint128_t(pid) << 32)) | current_time_point().sec_since_epoch();
            //o.id = pro_table.available_primary_key();
            o.pid = pid;
            o.seller_uid = product.uid;
            o.buyer_uid = buyer_uid;
            o.price = product.price;
            o.postage = product.postage;
            o.status = OrderStatus::OS_PENDING_PAYMENT;
            o.create_time = time_point_sec(current_time_point().sec_since_epoch());
            o.pay_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.pay_time_out);
        });
    }

    void bitsfleamain::payorder( uint128_t order_id, const asset& quantity )
    {
        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( order.price.symbol == quantity.symbol, "Invalid order symbol" );
        check( quantity.amount == (order.postage + order.price).amount, "Invalid order amount" );

        time_point_sec pay_time = time_point_sec(current_time_point().sec_since_epoch());
        check( pay_time < order.pay_time_out, "Order has expired");
        if( pay_time < order.pay_time_out ) {
            order_table.modify( order, same_payer, [&](auto& o){
                o.status = OrderStatus::OS_PENDING_SHIPMENT;
                o.pay_time = pay_time;
                o.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
            });
        } else {
            order_table.erase( order );
            check( false, "Order has expired");
        }
    }

    void bitsfleamain::OnEOSTransfer( const name& from, const name& to, const asset& quantity, const string& memo )
    {
        if( to != _self ) return;
        require_auth( from );
        if( quantity.symbol != SYS ) return;
        check( quantity.amount > 0, "Invalid quantity" );
        bool is_payorder = memo.find( string("payorder:") ) == 0;
        if( !is_payorder ) return;

        auto info = split( memo, ":" );
        uint128_t order_id = get_orderid( info[1] );

        payorder( order_id, quantity );
    }

    void bitsfleamain::reshipment( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& number )
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid buyer eosid" );
        check( number.length() <= 50, "number too long" );

        proreturn_index repro_table( _self, _self.value );
        auto& repro = repro_table.get( order_id, "invalid order id" );
        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( buyer_uid == order.buyer_uid, "This order does not belong to you" );
        check( repro.status == ReturnStatus::RS_PENDING_SHIPMENT, "This order is not ready for shipment" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        repro_table.modify( repro, same_payer, [&](auto& re){
            re.shipment_number = number;
            re.ship_time = current_time;
            re.status = ReturnStatus::RS_PENDING_RECEIPT;
            re.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
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
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( seller_uid == order.seller_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_SHIPMENT, "This order is not ready for shipment" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_table.modify( order, same_payer, [&](auto& o){
            o.shipment_number = number;
            o.ship_time = current_time;
            o.status = OrderStatus::OS_PENDING_RECEIPT;
            o.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
        });

        // shipment delivery timeout
        // if( current_time > order.ship_time_out ) {
        //     SubCredit( seller_uid, 5 );
        // }
    }

    void bitsfleamain::Settle( const Order& order, const User& seller, const User& buyer, const name& contract_name )
    {
        //eosio.token
        auto total = order.price + order.postage;
        auto income = asset( int64_t(double(total.amount) * _global.fee_ratio), total.symbol );
        auto amount = total - income;
        
        string memo = string("complete order ") + uint128ToString( order.id );
        action a1 = action( permission_level{_self, "active"_n}, contract_name, "transfer"_n,
            std::make_tuple( _self, seller.eosid, amount, memo )
        );
        if( buyer.referrer > 0 ){
            auto comm = asset( int64_t(double(income.amount) * _global.ref_commission_rate), income.symbol );
            if( comm.amount > 0 ) {
                income -= comm;
                auto& referrer = _user_table.get( buyer.referrer, "Invalid order referrer uid" );
                transaction trx;
                action a2 = action ( permission_level{_self, "active"_n}, contract_name, "transfer"_n,
                    std::make_tuple( _self, referrer.eosid, comm, "Referral commission" )
                );
                trx.actions.emplace_back( a1 );
                trx.actions.emplace_back( a2 );
                trx.delay_sec = 5;
                trx.send( (uint128_t(("settle"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
            } else {
                a1.send();
            }
        } else {
            a1.send();
        }

        vector<asset>::iterator itr = find_if( _global.income.begin(), _global.income.end(), [&](asset& a){
            return a.symbol == SYS;
        });
        if( itr != _global.income.end() ) {
            asset sys = (*itr) + income;
            _global.income.erase( itr );
            _global.income.push_back( sys );
        } else {
            _global.income.push_back( income );
        }
    }

    void bitsfleamain::endorder( const Order& order )
    {
        auto& seller = _user_table.get( order.seller_uid, "Invalid order seller_uid" );
        auto& buyer = _user_table.get( order.buyer_uid, "Invalid order buyer_uid" );
        _user_table.modify( seller, same_payer, [&](auto& u){
            u.sell_total += 1;
        });
        _user_table.modify( buyer, same_payer, [&](auto& u){
            u.buy_total += 1;
        });
        if( order.price.symbol == SYS ) { //EOS
            Settle( order, seller, buyer, "eosio.token"_n );
        }
    }

    void bitsfleamain::refund( const Order& order )
    {
        auto& buyer = _user_table.get( order.buyer_uid, "Invalid order seller_uid" );
        auto& seller = _user_table.get( order.seller_uid, "Invalid order seller_uid" );

        if( order.price.symbol == SYS ) { //EOS
            auto total = order.price + order.postage;
            //auto income = asset( uint64_t(double(order.price.amount) * _global.fee_ratio), order.price.symbol );
            string memo = string("returns order ") + uint128ToString( order.id );
            transaction trx;
            action a1 = action( permission_level{_self, "active"_n}, "eosio.token"_n, "transfer"_n,
                std::make_tuple( _self, buyer.eosid, order.price, memo )
            );
            action a2 =action( permission_level{_self, "active"_n}, "eosio.token"_n, "transfer"_n,
                std::make_tuple( _self, seller.eosid, order.postage, memo )
            );
            trx.actions.emplace_back( a1 );
            trx.actions.emplace_back( a2 );
            trx.delay_sec = 5;
            trx.send( (uint128_t(("refund"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
        }
    }

    void bitsfleamain::conreceipt( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id )
    {
        require_auth( buyer_eosid );

        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( buyer_uid == order.buyer_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_RECEIPT, "The order status is not OS_PENDING_RECEIPT" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_table.modify( order, same_payer, [&](auto& o){
            o.receipt_time = current_time;
            o.status = OrderStatus::OS_COMPLETED;
            o.end_time = current_time;
        });
        endorder( order );

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
            action a1 = action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
                std::make_tuple( buyer_eosid, point, string("transaction gift") )
            );
            action a2 = action(permission_level{_self, "active"_n}, "bitsfleamain"_n, "issue"_n,
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
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( seller_uid == order.seller_uid, "This order does not belong to you" );

        proreturn_index repro_table( _self, _self.value );
        auto& repro = repro_table.get( order_id, "invalid order id" );
        check( repro.status == ReturnStatus::RS_PENDING_RECEIPT, "The order status is not RS_PENDING_RECEIPT" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        repro_table.modify( repro, same_payer, [&](auto& re){
            re.receipt_time = current_time;
            re.status = ReturnStatus::RS_COMPLETED;
            re.end_time = current_time;
        });
        //Refund
        refund( order );
        // receipt delivery timeout
        if( current_time > order.receipt_time_out ) {
            SubCredit( seller_uid, 5 );
        }
    }

    void bitsfleamain::deferreceipt( uint64_t user_uid, const name& user_eosid, uint128_t order_id )
    {
        require_auth( user_eosid );

        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( order.buyer_uid == user_uid, "This order does not belong to you" );
        check( order.delayed_count < 3, "Has been postponed three times" );

        order_table.modify( order, same_payer, [&](auto& o){
            o.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            o.delayed_count += 1;
        });
    }

    void bitsfleamain::deferreturn( uint64_t user_uid, const name& user_eosid, uint128_t order_id )
    {
        require_auth( user_eosid );

        order_index order_table( _self, _self.value );
        proreturn_index proreturn_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        auto& proreturn = proreturn_table.get( order_id, "Invalid order id for returns table" );
        check( order.seller_uid == user_uid, "This order does not belong to you" );
        check( proreturn.delayed_count < 3, "Has been postponed three times" );

        proreturn_table.modify( proreturn, same_payer, [&](auto& pr){
            pr.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            pr.delayed_count += 1;
        });

    }

    void bitsfleamain::returns( uint64_t buyer_uid, const name& buyer_eosid, uint128_t order_id, const string& reasons )
    {
        require_auth( buyer_eosid );

        check( is_account( buyer_eosid ), "Invalid buyer eosid" );
        check( reasons.length() <= 300, "reasons too long" );

        order_index order_table( _self, _self.value );
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( order.buyer_uid == buyer_uid, "Invalid buyer uid" );
        check( order.status == OrderStatus::OS_PENDING_RECEIPT, "product already received can be returned" );

        //is_returns
        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( order.pid, "Invalid product id" );
        check( product.is_returns, "This item does not support returns" );

        order_table.modify( order, same_payer, [&](auto& o){
            o.status = OrderStatus::OS_RETURN;
        });

        proreturn_index res_table( _self, _self.value );
        res_table.emplace( _self, [&](auto& r){
            r.id = res_table.available_primary_key();
            r.order_id = order_id;
            r.pid = order.pid;
            r.order_price = order.price;
            r.status = ReturnStatus::RS_PENDING_SHIPMENT;
            r.reasons = reasons;
            r.create_time = time_point_sec(current_time_point().sec_since_epoch());
            r.ship_time_out = time_point_sec(current_time_point().sec_since_epoch() + _global.ship_time_out);
        });

    }
    

    
}