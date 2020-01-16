cleos wallet unlock -n localtest  --password PW5JAipZzmGEfah7rLgi6hwyw8Hrg6yxCLxtBjEXXiu9GQ51dZZ3y

cleos create account eosio eosio.token EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio bitsfleamain EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player1 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player2 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos set contract eosio /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.bios  -p eosio &&

cleos set contract eosio.token /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.token -p eosio.token &&

cleos set contract bitsfleamain ./build/bitsfleamain -p bitsfleamain &&

cleos push action eosio.token create '[ "eosio", "1000000000.0000 EOS", 0, 0, 0]' -p eosio.token &&

cleos push action bitsfleamain init '[]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player,"player","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",0]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player1,"player1","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",0]' -p bitsfleamain &&

cleos push action bitsfleamain reguser '[player2,"player2","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B494","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",1]' -p bitsfleamain &&

cleos get table bitsfleamain bitsfleamain users # &&