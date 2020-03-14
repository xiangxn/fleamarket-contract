
cleos push action bitsfleamain bindaddr '[0,"player","8,NULS","tNULSeBaMhVijVH57U7EkDhERASy9jhtZSBsfg"]' -p player &&

#cleos push action bitsfleamain issue '[ "bitsfleamain", "100.00000000 NULS", "memo" ]' -p bitsfleamain &&

#zcleos push action bitsfleamain issue '[ "player", "100.00000000 NULS", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain issue '[ "player1", "100.00000000 NULS", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain issue '[ "fleagateways", "100.00000000 NULS", "memo" ]' -p bitsfleamain &&

#cleos transfer player bitsfleamain "1.00000000 NULS" "w:" -p player -c bitsfleamain &&

cleos push action bitsfleamain publish '[0,{"pid":0,"uid":0,"title":"title test NULS","description":"description 测试NULS","photos":"photos 测试","category":1,"status":0,"is_new":false,"is_returns":true,"reviewer":0,"sale_method":0,"price":"2.09000000 NULS","transaction_method":1,"postage":"0.01000000 NULS","position":"位置","release_time":"2020-01-20T00:00:00"},{"id":0,"pid":0,"security":"1.00000000 NULS","markup":"1.0000 NULS","current_price":"1.00000000 NULS","auction_times":0,"last_price_user":0,"start_time":"2020-01-20T00:00:00","end_time":"2020-01-22T00:00:00"}]' -p bitsfleamain &&

cleos push action bitsfleamain review '[1,"reviewer",1,false,"good product"]' -p reviewer &&

cleos push action bitsfleamain placeorder '[2,"player1",1,"0"]' -p player1 &&

cleos get currency balance bitsfleamain player NULS && 

cleos get table bitsfleamain bitsfleamain orders &&

cleos get table bitsfleamain bitsfleamain othersettle

# pay : order id 在上一行的输出中取
# cleos push action bitsfleamain shipment '[0,"player","36893488153298263397","229988222"]' -p player
# cleos push action bitsfleamain conreceipt '[2,"player1","36893488153298263397"]' -p player1
# cleos get currency balance bitsfleamain player NULS
# cleos get currency balance bitsfleamain player1 NULS
# cleos get currency balance bitsfleamain bitsfleamain NULS
# cleos get currency balance bitsfleamain reviewer NULS
# cleos get currency balance bitsfleamain fleadevopsac NULS