#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/crypto.hpp>
#include <string>

#include "types.hpp"
#include "../../../tools/include/tool.hpp"

using namespace std;
using namespace eosio;

namespace rareteam
{
    enum ProductStatus : uint32_t {
        PUBLISH = 0,
        NORMAL = 100,
        COMPLETED = 200,
        DELISTED = 300,
        LOCKED = 400
    };

    enum OrderStatus : uint32_t {
        OS_PENDING_PAYMENT = 0,
        OS_PENDING_CONFIRM = 100,
        OS_CANCELLED = 200,
        OS_PENDING_SHIPMENT = 300,
        OS_PENDING_RECEIPT = 400,
        OS_PENDING_SETTLE = 500,
        OS_COMPLETED = 600,
        OS_ARBITRATION = 700,
        OS_RETURN = 800
    };

    enum ReturnStatus : uint32_t {
        RS_PENDING_SHIPMENT = 0,
        RS_PENDING_RECEIPT = 100,
        RS_COMPLETED = 200,
        RS_CANCELLED = 300,
        RS_ARBITRATION = 400
    };

    enum OtherSettleStatus : uint32_t {
        OSS_NORMAL = 0,
        OSS_PAID = 100
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Categories
    {
        uint16_t id = 0;
        string view;
        uint16_t parent = 0;

        uint64_t primary_key() const { return uint64_t(id); }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Product
    {
        uint32_t pid;
        uint64_t uid = 0;
        string title;
        string description;
        vector<string> photos;
        uint64_t category;
        uint32_t status = ProductStatus::PUBLISH;
        bool is_new = false;
        bool is_returns = false;
        uint64_t reviewer = 0;
        /**
         * 0 Buy Now, 1 Auction
         */ 
        uint8_t sale_method = 0;
        asset price;
        /**
         * 0 Pick up and 1 mail
         */ 
        uint8_t transaction_method = 0;
        /**
         * Stock quantity
         */ 
        uint32_t stock_count = 0;
        /**
         * Whether retail
         */ 
        bool is_retail = false;

        asset postage;
        string position;
        time_point_sec release_time;

        uint64_t primary_key() const { return uint64_t(pid); }
        uint64_t by_status() const { return uint64_t(status); }
        uint64_t by_uid() const { return uid; }

    };

    //auction
    struct [[eosio::table, eosio::contract("bitsfleamain")]] ProductAuction
    {
        uint64_t id;
        uint32_t pid;
        /**
         * security deposit
         */ 
        asset security;
        asset markup;
        asset current_price;
        uint32_t auction_times;
        uint64_t last_price_user = 0;
        time_point_sec start_time;
        time_point_sec end_time;

        uint64_t primary_key() const { return id; }
        uint64_t by_pid() const { return uint64_t(pid); }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Order
    {
        uint64_t id;
        uint128_t oid;
        uint32_t pid;
        uint64_t seller_uid;
        uint64_t buyer_uid;
        asset price;
        asset postage;
        uint32_t status = OrderStatus::OS_PENDING_PAYMENT;
        string shipment_number = "";
        string pay_addr = "";
        time_point_sec create_time;
        time_point_sec pay_time;
        time_point_sec pay_time_out;
        time_point_sec ship_time;
        time_point_sec ship_time_out;
        time_point_sec receipt_time;
        time_point_sec receipt_time_out;
        time_point_sec end_time;
        /**
         * Delayed receipts times 
         */ 
        uint32_t delayed_count = 0;

        uint32_t to_addr = 0;

        uint64_t primary_key() const { return id; }
        uint64_t by_pid() const { return uint64_t(pid); }
        uint64_t by_seller() const { return seller_uid; }
        uint64_t by_buyer() const { return buyer_uid; }
        uint64_t by_time() const {return uint64_t((oid<<96)>>96); }
        uint128_t by_oid() const {return oid;}
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] ProReturn
    {
        uint32_t id;
        uint128_t order_id;
        uint32_t pid;
        asset order_price;
        uint32_t status = ReturnStatus::RS_PENDING_SHIPMENT;
        string reasons = "";
        string shipment_number = "";
        time_point_sec create_time;
        time_point_sec ship_time;
        time_point_sec ship_time_out;
        time_point_sec receipt_time;
        time_point_sec receipt_time_out;
        time_point_sec end_time;
        /**
         * Delayed receipts times 
         */ 
        uint32_t delayed_count = 0;
        uint32_t to_addr = 0;

        uint64_t primary_key() const { return uint64_t(id); }
        uint128_t by_order_id() const { return order_id; }
        uint64_t by_pid() const { return uint64_t(pid); }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] OtherSettle
    {
        uint64_t id;
        uint64_t uid;
        uint128_t order_id;
        asset amount;
        uint32_t status = OtherSettleStatus::OSS_NORMAL;
        string addr = "";
        string memo = "";
        string trx_id = "";
        time_point_sec start_time;
        time_point_sec end_time;

        uint64_t primary_key() const { return id; }
        uint64_t by_uid() const { return uid; }
    };

    typedef eosio::multi_index<"categories"_n, Categories > Category_index;
    typedef eosio::multi_index<"products"_n, Product,
        indexed_by< "status"_n, const_mem_fun<Product, uint64_t, &Product::by_status> >,
        indexed_by< "byuid"_n, const_mem_fun<Product, uint64_t, &Product::by_uid> >
    > product_index;

    typedef eosio::multi_index<"proauction"_n, ProductAuction,
        indexed_by< "bypid"_n, const_mem_fun<ProductAuction, uint64_t,  &ProductAuction::by_pid> >
    > auction_index;

    typedef eosio::multi_index<"orders"_n, Order,
        indexed_by< "orderpid"_n, const_mem_fun<Order, uint64_t,  &Order::by_pid> >,
        indexed_by< "byseller"_n, const_mem_fun<Order, uint64_t,  &Order::by_seller> >,
        indexed_by< "bybuyer"_n, const_mem_fun<Order, uint64_t,  &Order::by_buyer> >,
        indexed_by< "bytime"_n, const_mem_fun<Order, uint64_t, &Order::by_time> >,
        indexed_by< "byoid"_n, const_mem_fun<Order, uint128_t, &Order::by_oid> >
    > order_index;

    typedef eosio::multi_index<"returns"_n, ProReturn,
        indexed_by< "bypid"_n, const_mem_fun<ProReturn, uint64_t,  &ProReturn::by_pid> >,
        indexed_by< "byorderid"_n, const_mem_fun<ProReturn, uint128_t,  &ProReturn::by_order_id> >
    > proreturn_index;

    typedef eosio::multi_index<"othersettle"_n, OtherSettle,
        indexed_by< "byuid"_n, const_mem_fun<OtherSettle, uint64_t,  &OtherSettle::by_uid> >
    > othersettle_index;


    uint128_t get_orderid( const string& order_id_str )
    {
        uint128_t order_id = 0;
        if( order_id_str.substr(0,2) == "0x" ) {
            check( order_id_str.length() == 34, "Invalid order id");
            from_hex( order_id_str.substr(2), reinterpret_cast<char*>(&order_id), sizeof(order_id) );
            return order_id;
        } else {
            order_id = uint128FromString( order_id_str );
        }
        return order_id;
    };

    uint64_t get_buyer_uid_by_orderid( const uint128_t& orderid )
    {
        //to_uint64
        return uint64_t(orderid >> 64);
    };

    uint32_t get_pid_by_orderid( const uint128_t& orderid )
    {
        //to_uint32
        return uint32_t((orderid << 64) >> 96);
    };
    

} // namespace egame
