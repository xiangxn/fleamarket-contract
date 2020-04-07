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
        check( IsLockUser( user ) == false, "Account is locked" );
        check( product.price.symbol == product.postage.symbol, "Inconsistent payment methods" );
        check( CheckSymbol( product.price.symbol ), "This currency is not currently supported" );

        Category_index category_table( _self, _self.value);
        auto cat_itr = category_table.find( product.category );
        check( cat_itr != category_table.end(), "Invalid product category" );

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
            AddTableLog("products"_n, OpType::OT_INSERT, p.pid );
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
                AddTableLog( "proauction"_n, OpType::OT_INSERT, pa_item.id );
            });
        }
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

    void bitsfleamain::review( uint64_t reviewer_uid, const name& reviewer_eosid, uint32_t pid, bool is_delisted, string& memo )
    {
        require_auth( reviewer_eosid );
        check( IsLockUser( reviewer_uid ) == false, "Account is locked" );

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
                AddTableLog( "proaudits"_n, OpType::OT_INSERT, a.id );
            });

            pro_table.modify( pro_itr, same_payer, [&]( auto& p ){
                if( is_delisted ){
                    p.status = ProductStatus::DELISTED;
                    SubCredit( pro_itr->uid, 5 );//delisted sub 5 credit 
                } else {
                    p.status = ProductStatus::NORMAL;
                }
                p.reviewer = reviewer_uid;
                AddTableLog("products"_n, OpType::OT_UPDATE, p.pid );
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
                AddTableLog("users"_n, OpType::OT_UPDATE, publisher.uid);
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
            AddTableLog( "proauction"_n, OpType::OT_UPDATE, a.id );
        });
    }

    void bitsfleamain::placeorder( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const uint128_t& order_id)
    {
        require_auth( buyer_eosid );
        if( order_id > 0 ) {
            require_auth( _self );
        }
        check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
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
        auto order_itr = order_table.find( oid );
        check( order_itr == order_table.end(), "Order number already exists" );
        //create order
        //fixed price
        order_table.emplace( _self, [&](auto& o){
            o.id = oid;
            //o.id = pro_table.available_primary_key();
            o.pid = pid;
            o.seller_uid = product.uid;
            o.buyer_uid = buyer_uid;
            o.price = product.price;
            o.postage = product.postage;
            o.status = OrderStatus::OS_PENDING_PAYMENT;
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
        auto& order = order_table.get( order_id, "PayOrder:Invalid order id" );
        check( order.price.symbol == quantity.symbol, "Invalid order symbol" );
        check( quantity.amount == (order.postage + order.price).amount, "Invalid order amount" );
        check( order.status == OrderStatus::OS_PENDING_PAYMENT, "Invalid order status");

        time_point_sec pay_time = time_point_sec(current_time_point().sec_since_epoch());
        check( pay_time < order.pay_time_out, "Order has expired");
        if( pay_time < order.pay_time_out ) {
            order_table.modify( order, same_payer, [&](auto& o){
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
            check( false, "Order has expired");
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

    void bitsfleamain::OnMyTransfer( const name& from, const name& to, const asset& quantity, const string& memo )
    {
        if( to != _self ) return;
        check( quantity.amount > 0, "Invalid quantity" );
        if( CheckSymbol( quantity.symbol ) == false ) return;
        bool is_payorder = memo.find( "p:" ) == 0;
        bool is_withdraw = memo.find( "w:" ) == 0;
        if( !is_payorder && !is_withdraw ) return;

        auto info = split( memo, ":" );
        auto payer = from;
        if( info.size() > 2 ){
            payer = name(info[2]);
        }
        if( is_payorder ) {
            uint128_t order_id = get_orderid( info[1] );
            PayOrder( order_id, quantity );
        } else if( is_withdraw ) {
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
                auto oid = os_table.available_primary_key();
                os.id = oid == 0 ? 1 : oid;
                os.uid = user_itr->uid;
                os.order_id = 0;
                os.amount = quantity - coin.fee; //Withdrawal fees have been deducted
                os.status = OtherSettleStatus::OSS_NORMAL;
                os.addr = addr;
                os.memo = "withdraw coin";
                os.start_time = time_point_sec(current_time_point().sec_since_epoch());
            });
            sub_balance( _self, coin.fee );
            add_balance( _global.gateway, coin.fee, _self );
        }
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
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( seller_uid == order.seller_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_SHIPMENT, "This order is not ready for shipment" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_table.modify( order, same_payer, [&](auto& o){
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
                    auto oid = os_table.available_primary_key();
                    os.id = oid == 0 ? 1 : oid;
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
                    action a1 = action( permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, ACTION_NAME_TRANSFER,
                        std::make_tuple( _self, _global.devops, devops_income, memo )
                    );
                    trx.actions.emplace_back( a1 );
                }
                if( buyer.referrer > 0 && comm.amount > 0 ){
                    auto& referrer = _user_table.get( buyer.referrer, "Invalid order referrer uid" );
                    action a2 = action( permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, ACTION_NAME_TRANSFER,
                        std::make_tuple( _self, referrer.eosid, comm, "Referral commission" + stroid )
                    );
                    trx.actions.emplace_back( a2 );
                }
                if( trx.actions.size() > 0 ) {
                    trx.delay_sec = 5;
                    trx.send( (uint128_t(("settle"_n).value) << 64) | uint64_t(current_time_point().sec_since_epoch()) , _self, false);
                }
            } else { // bitsfleamain transfer
                PayCoin( stroid, order, seller, buyer, amount, devops_income, comm, FLEA_PLATFORM );
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
        action( permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, ACTION_NAME_TRANSFER,
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
            action a3 = action( permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, ACTION_NAME_TRANSFER,
                std::make_tuple( _self, buyer.eosid, order.price, memo )
            );
            action a4 = action( permission_level{_self, ACTIVE_PERMISSION}, FLEA_PLATFORM, ACTION_NAME_TRANSFER,
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
        auto& order = order_table.get( order_id, "Invalid order id" );
        check( buyer_uid == order.buyer_uid, "This order does not belong to you" );
        check( order.status == OrderStatus::OS_PENDING_RECEIPT, "The order status is not OS_PENDING_RECEIPT" );

        time_point_sec current_time = time_point_sec(current_time_point().sec_since_epoch());
        order_table.modify( order, same_payer, [&](auto& o){
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
            AddTableLog( "returns"_n, OpType::OT_UPDATE, re.id );
        });
        //Refund
        Refund( order );
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
        check( order.delayed_count < _global.max_deferr_times, "Has been postponed three times" );

        order_table.modify( order, same_payer, [&](auto& o){
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
        auto& order = order_table.get( order_id, "Invalid order id" );
        auto& proreturn = proreturn_table.get( order_id, "Invalid order id for returns table" );
        check( order.seller_uid == user_uid, "This order does not belong to you" );
        check( proreturn.delayed_count < _global.max_deferr_times, "Has been postponed three times" );

        proreturn_table.modify( proreturn, same_payer, [&](auto& pr){
            pr.receipt_time_out = time_point_sec( current_time_point().sec_since_epoch() + _global.receipt_time_out );
            pr.delayed_count += 1;
            AddTableLog( "returns"_n, OpType::OT_UPDATE, pr.id );
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
            AddTableLog( "orders"_n, OpType::OT_UPDATE, o.id );
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
            AddTableLog( "returns"_n, OpType::OT_INSERT, r.id );
        });

    }
    

    
}