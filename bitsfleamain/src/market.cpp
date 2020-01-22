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
        //TODO: point logic
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
                } else {
                    p.status = ProductStatus::NORMAL;
                }
            });
        }
        //TODO: point logic
    }

    void bitsfleamain::bidauction( uint64_t buyer_uid, const name& buyer_eosid, uint32_t pid, const asset& price )
    {
        require_auth( buyer_eosid );
        check( is_account( buyer_eosid ), "Invalid account buyer_eosid" );
        auto& user = _user_table.get( buyer_uid, "Invalid account buyer_uid" );

        product_index pro_table( _self, _self.value );
        auto& product = pro_table.get( pid, "Invalid product pid" );
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
        //TODO: point logic
    }
    

    
}