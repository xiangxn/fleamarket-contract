cleos push action bitsfleamain appreviewer '[1,"reviewer"]' -p reviewer &&

cleos push action bitsfleamain votereviewer '[2,"player1",1,true]' -p player1 &&

cleos push action bitsfleamain publish '[0,{"pid":0,"uid":0,"title":"title test","description":"description 测试","photos":"photos 测试","category":0,"status":0,"is_new":false,"is_returns":true,"reviewer":0,"sale_method":0,"price":"100.0000 EOS","transaction_method":1,"postage":"1.0000 EOS","position":"位置","release_time":"2020-01-20T00:00:00"},{"id":0,"pid":0,"security":"1.0000 EOS","markup":"1.0000 EOS","current_price":"1.0000 EOS","auction_times":0,"last_price_user":0,"start_time":"2020-01-20T00:00:00","end_time":"2020-01-22T00:00:00"}]' -p bitsfleamain &&

cleos push action bitsfleamain review '[1,"reviewer",0,false,"good product"]' -p reviewer &&

cleos push action bitsfleamain placeorder '[2,"player1",0]' -p player1 &&

cleos get table bitsfleamain bitsfleamain orders 

# pay : order id 在上一行的输出中取
# cleos transfer player1 bitsfleamain "101.0000 EOS" "payorder:0x08563e5e000000000200000000000000" -p player1
# cleos get currency balance eosio.token player1 EOS
# cleos get currency balance eosio.token bitsfleamain EOS
# cleos push action bitsfleamain shipment '[0,"player",0x08563e5e000000000200000000000000,"229988222"]' -p player
# cleos push action bitsfleamain conreceipt '[2,"player1",0x08563e5e000000000200000000000000]' -p player1
# cleos push action bitsfleamain returns '[2,"player1",0x08563e5e000000000200000000000000,"商品与描述不符"]' -p player1
# cleos push action bitsfleamain reshipment '[2,"player1",0x08563e5e000000000200000000000000,"229988223"]' -p player1
# cleos push action bitsfleamain reconreceipt '[0,"player",0x08563e5e000000000200000000000000]' -p player

cleos get table bitsfleamain bitsfleamain returns

# cleos get currency balance eosio.token player EOS
# cleos get currency balance bitsfleamain player1 FMP
# cleos get currency balance bitsfleamain reviewer FMP
# cleos push action bitsfleamain transfer '["reviewer","player","1.0000 FMP",""]' -p reviewer
