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
    enum ProductStatus : uint8_t {
        PUBLISH = 0,
        NORMAL,
        COMPLETED,
        DELISTED,
        LOCKED
    };

    enum OrderStatus : uint8_t {
        OS_PENDING_PAYMENT = 0,
        OS_PAID,
        OS_PENDING_SHIPMENT,
        OS_SHIPPED,
        OS_PENDING_RECEIPT,
        OS_RECEIVED,
        OS_COMPLETED,
        OS_CANCELLED,
        OS_ARBITRATION,
        OS_RETURN
    };

    enum ReturnStatus : uint8_t {
        RS_PENDING_SHIPMENT = 0,
        RS_SHIPPED,
        RS_PENDING_RECEIPT,
        RS_COMPLETED,
        RS_CANCELLED,
        RS_ARBITRATION
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Categories
    {
        uint64_t id = 0;
        name key;
        string view;

        uint64_t primary_key() const { return id; }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Product
    {
        uint64_t pid;
        string title;
        string description;
        string photos;
        uint64_t category;
        uint8_t status = ProductStatus::PUBLISH;
        bool is_new = false;
        bool is_returns = false;
        /**
         * 0 Buy Now, 1 Auction
         */ 
        uint8_t sale_method = 0;
        asset price;
        /**
         * 0 Pick up and 1 mail
         */ 
        uint8_t transaction_method = 0;

        asset postage;
        string position;
        time_point_sec release_time;

        uint64_t primary_key() const { return pid; }
        uint64_t by_status() const { return uint64_t(status); }

    };

    //auction
    struct [[eosio::table, eosio::contract("bitsfleamain")]] ProductAuction
    {
        uint64_t id;
        uint64_t pid;
        /**
         * security deposit
         */ 
        asset security;
        uint32_t markup = 0;
        asset current_price;
        uint32_t auction_times;
        uint64_t last_price_user = 0;
        time_point_sec start_time;
        time_point_sec end_time;

        uint64_t primary_key() const { return pid; }
        uint64_t by_auction_id() const { return id; }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] Order
    {
        uint64_t id;
        uint64_t pid;
        uint64_t seller_uid;
        uint64_t buyer_uid;
        asset price;
        uint8_t status = OrderStatus::OS_PENDING_PAYMENT;
        string shipment_number;
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
        uint32_t delayed_count;

        uint64_t primary_key() const { return id; }
        uint64_t by_pid() const { return pid; }
    };

    struct [[eosio::table, eosio::contract("bitsfleamain")]] ProReturn
    {
        uint64_t id;
        uint64_t order_id;
        uint64_t pid;
        asset order_price;
        uint8_t status = ReturnStatus::RS_PENDING_SHIPMENT;
        string shipment_number;
        time_point_sec create_time;
        time_point_sec ship_time;
        time_point_sec ship_time_out;
        time_point_sec receipt_time;
        time_point_sec receipt_time_out;
        time_point_sec end_time;
        /**
         * Delayed receipts times 
         */ 
        uint32_t delayed_count;

        uint64_t primary_key() const { return order_id; }
        uint64_t by_rid() const { return id; }
        uint64_t by_pid() const { return pid; }
    };

    typedef eosio::multi_index<"categories"_n, Categories > Category_index;
    typedef eosio::multi_index<"products"_n, Product,
        indexed_by< "status"_n, const_mem_fun<Product, uint64_t, &Product::by_status> >
    > product_index;

    typedef eosio::multi_index<"proauction"_n, ProductAuction,
        indexed_by< "auctionid"_n, const_mem_fun<ProductAuction, uint64_t,  &ProductAuction::by_auction_id> >
    > auction_index;

    typedef eosio::multi_index<"orders"_n, Order,
        indexed_by< "orderpid"_n, const_mem_fun<Order, uint64_t,  &Order::by_pid> >
    > order_index;

    typedef eosio::multi_index<"returns"_n, ProReturn,
        indexed_by< "returnpid"_n, const_mem_fun<ProReturn, uint64_t,  &ProReturn::by_pid> >,
        indexed_by< "returnrid"_n, const_mem_fun<ProReturn, uint64_t,  &ProReturn::by_rid> >
    > proreturn_index;
    

} // namespace egame
