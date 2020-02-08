cleos wallet unlock -n localtest  --password PW5JAipZzmGEfah7rLgi6hwyw8Hrg6yxCLxtBjEXXiu9GQ51dZZ3y

cleos create account eosio eosio.token EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio bitsfleamain EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio reviewer EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player1 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos set contract eosio /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.bios  -p eosio &&

cleos set contract eosio.token /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.token -p eosio.token &&

cleos set contract bitsfleamain ./build/bitsfleamain -p bitsfleamain &&

cleos push action eosio.token create '[ "eosio", "1000000000.0000 EOS", 0, 0, 0]' -p eosio.token &&

cleos set account permission bitsfleamain active '{"threshold": 1,"keys": [{"key": "EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ","weight": 1}],"accounts": [{"permission":{"actor":"bitsfleamain","permission":"eosio.code"},"weight":1}]}' owner -p bitsfleamain &&

cleos push action bitsfleamain init '[]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player,"player","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",0]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[reviewer,"reviewer","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493",0]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player1,"player1","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B494","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493",1]' -p bitsfleamain &&

cleos push action eosio.token issue '[ "player", "10000.0000 EOS", "memo" ]' -p eosio &&
cleos push action eosio.token issue '[ "player1", "10000.0000 EOS", "memo" ]' -p eosio &&
cleos push action eosio.token issue '[ "reviewer", "10000.0000 EOS", "memo" ]' -p eosio &&

cleos get table bitsfleamain bitsfleamain global &&

# cleos get currency stats bitsfleamain FMP &&
cleos get table bitsfleamain FMP stat &&

cleos get table bitsfleamain bitsfleamain users # &&

# {"uid":0,"product":{"pid":0,"title":"title test","description":"description 测试","photos":"photos 测试","category":0,"status":0,"is_new":false,"is_returns":false,"sale_method":0,"price":"100.0000 EOS","transaction_method":1,"postage":"1.0000 EOS","position":"位置"},"pa":null}