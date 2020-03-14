
cleos push action bitsfleamain bindaddr '[0,"player","8,ETH","0x23Ee6406969A9739E695C7b0C6b66daF47b8d782"]' -p player &&

cleos push action bitsfleamain issue '[ "bitsfleamain", "100.00000000 ETH", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain issue '[ "player", "100.00000000 ETH", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain issue '[ "player1", "100.00000000 ETH", "memo" ]' -p bitsfleamain &&

cleos transfer player bitsfleamain "0.10000000 ETH" "w:" -p player -c bitsfleamain &&

cleos push action bitsfleamain publish '[0,{"pid":0,"uid":0,"title":"title test ETH","description":"description 测试ETH","photos":"photos 测试","category":1,"status":0,"is_new":false,"is_returns":true,"reviewer":0,"sale_method":0,"price":"0.0900 ETH","transaction_method":1,"postage":"0.0100 ETH","position":"位置","release_time":"2020-01-20T00:00:00"},{"id":0,"pid":0,"security":"1.0000 ETH","markup":"1.0000 ETH","current_price":"1.0000 ETH","auction_times":0,"last_price_user":0,"start_time":"2020-01-20T00:00:00","end_time":"2020-01-22T00:00:00"}]' -p bitsfleamain &&

cleos push action bitsfleamain review '[1,"reviewer",1,false,"good product"]' -p reviewer &&

cleos push action bitsfleamain placeorder '[2,"player1",1,"0"]' -p player1 &&

cleos get currency balance bitsfleamain player ETH &&

cleos get table bitsfleamain bitsfleamain othersettle