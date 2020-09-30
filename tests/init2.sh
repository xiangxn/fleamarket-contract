cleos push action bitsfleamain init "[]" -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player,"player","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",0, "EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW"]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[reviewer,"reviewer","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493",0, "EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW"]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player1,"player1","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B494","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493",1, "EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW"]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[reviewer1111,"reviewer1111","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B495","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B495",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain &&

cleos push action bitsfleamain appreviewer '[3,"reviewer1111"]' -p reviewer1111 &&

cleos push action eosio.token issue '[ "player", "10000.0000 BOS", "memo" ]' -p eosio &&
cleos push action eosio.token issue '[ "player1", "10000.0000 BOS", "memo" ]' -p eosio &&
cleos push action eosio.token issue '[ "reviewer", "10000.0000 BOS", "memo" ]' -p eosio &&
cleos push action eosio.token issue '[ "fleafaucet11", "10000.0000 BOS", "memo" ]' -p eosio &&


cleos get table bitsfleamain bitsfleamain global &&

cleos get table bitsfleamain bitsfleamain coins &&

# cleos get currency stats bitsfleamain FMP &&
cleos get table bitsfleamain FMP stat &&

cleos get table bitsfleamain bitsfleamain users # &&

# {"uid":0,"product":{"pid":0,"title":"title test","description":"description 测试","photos":"photos 测试","category":0,"status":0,"is_new":false,"is_returns":false,"sale_method":0,"price":"100.0000 BOS","transaction_method":1,"stock_count":1,"is_retail":false,"postage":"1.0000 BOS","position":"位置"},"pa":null}