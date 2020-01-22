cleos push action bitsfleamain appreviewer '[1,"reviewer"]' -p reviewer &&

cleos push action bitsfleamain votereviewer '[2,"player1",1,true]' -p player1 &&

cleos push action bitsfleamain publish '[0,{"pid":0,"uid":0,"title":"title test","description":"description 测试","photos":"photos 测试","category":0,"status":0,"is_new":false,"is_returns":false,"sale_method":0,"price":"100.0000 EOS","transaction_method":1,"postage":"1.0000 EOS","position":"位置","release_time":"2020-01-20T00:00:00"},{"id":0,"pid":0,"security":"1.0000 EOS","markup":"1.0000 EOS","current_price":"1.0000 EOS","auction_times":0,"last_price_user":0,"start_time":"2020-01-20T00:00:00","end_time":"2020-01-22T00:00:00"}]' -p bitsfleamain &&

cleos push action bitsfleamain review '[1,"reviewer",1,false,"good product"]' -p reviewer &&

cleos push action bitsfleamain placeorder '[2,"player1",1]' -p player1 &&

cleos get table bitsfleamain bitsfleamain orders