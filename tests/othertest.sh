cleos push action bitsfleamain issue '[ "player1", "10000.0000 BTS", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain publish '[0,{"pid":0,"uid":0,"title":"title test","description":"description 测试","photos":"photos 测试","category":0,"status":0,"is_new":false,"is_returns":true,"reviewer":0,"sale_method":0,"price":"100.0000 BTS","transaction_method":1,"postage":"1.0000 BTS","position":"位置","release_time":"2020-01-20T00:00:00"},{"id":0,"pid":0,"security":"1.0000 BTS","markup":"1.0000 BTS","current_price":"1.0000 BTS","auction_times":0,"last_price_user":0,"start_time":"2020-01-20T00:00:00","end_time":"2020-01-22T00:00:00"}]' -p bitsfleamain &&

cleos push action bitsfleamain review '[1,"reviewer",1,false,"good product"]' -p reviewer &&

cleos push action bitsfleamain placeorder '[2,"player1",1]' -p player1 &&

cleos get table bitsfleamain bitsfleamain orders 

# pay : order id 在上一行的输出中取
# cleos transfer player1 bitsfleamain "101.0000 BTS" "payorder:0x165a3e5e010000000200000000000000" -p player1 -c bitsfleamain
# cleos get currency balance bitsfleamain player BTS
# cleos get currency balance bitsfleamain player1 BTS
# cleos get currency balance bitsfleamain bitsfleamain BTS

# cleos push action bitsfleamain shipment '[0,"player",0x165a3e5e010000000200000000000000,"229988222"]' -p player
# cleos push action bitsfleamain conreceipt '[2,"player1",0x165a3e5e010000000200000000000000]' -p player1
# cleos push action bitsfleamain returns '[2,"player1",0x165a3e5e010000000200000000000000,"商品与描述不符"]' -p player1
# cleos push action bitsfleamain reshipment '[2,"player1",0x165a3e5e010000000200000000000000,"229988223"]' -p player1
# cleos push action bitsfleamain reconreceipt '[0,"player",0x165a3e5e010000000200000000000000]' -p player

# cleos push action bitsfleamain bindaddr '[0,"player","4,BTS","necklace"]' -p player
# cleos transfer player bitsfleamain "50.0000 BTS" "withdraw:0x165a3e5e0100000" -p player -c bitsfleamain
# cleos push action bitsfleamain closesettle '[0]' -p bitsfleamain
# cleos get table bitsfleamain bitsfleamain othersettle

cleos get currency balance bitsfleamain player BTS &&
cleos get currency balance bitsfleamain player1 BTS &&
cleos get currency balance bitsfleamain reviewer BTS &&
cleos get currency balance bitsfleamain bitsfleamain BTS