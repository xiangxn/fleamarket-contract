cleos wallet unlock -n localtest  --password PW5JAipZzmGEfah7rLgi6hwyw8Hrg6yxCLxtBjEXXiu9GQ51dZZ3y

cleos create account eosio eosio.token EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio bitsfleamain EOS76g5Xb1qHiv6iqzLCix2wsgSYzAAAnbCXMhU85n4SZPibqD8bV EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW &&

cleos create account eosio fleadevopsac EOS8PKxnqjuf9hpK17BDUEc33QD5d894KF9SZjiLSc7bqKVXGGhfy EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo &&

cleos create account eosio player EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio reviewer EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio player1 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio fleagateways EOS6xMD89KCnT92hYqjk1DBTwP5GtpwkjrgDGuHTi4CS4w1tFKT9j EOS5ix3Mo2rbDwWSQyTAKtYSmEapuxxyHmk6gsGuWNUJBsGA5s7si &&

cleos create account eosio fleafaucet11 EOS5p5d2xA9tLVLc8knw1afQfpfBXuHcFZGBVDUVVCy677LBm3K9k EOS6RbTLtFQ49MKa8epucQT7FvTjcCHgLc58FzY9mcPVcN94omxtT &&

cleos create account eosio necklace EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos create account eosio reviewer1111 EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ EOS8M1jvYGgV2XuPqS2nbZaW9tRZfUjT1N88BxJVeF4jVFDLfSAzQ &&

cleos set contract eosio /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.bios  -p eosio &&

cleos set contract eosio.token /Users/necklace/work/eos/eosio.contracts/build/contracts/eosio.token -p eosio.token &&

cleos set contract bitsfleamain ./build/bitsfleamain -p bitsfleamain &&

cleos push action eosio.token create '[ "eosio", "1000000000.0000 BOS", 0, 0, 0]' -p eosio.token &&

cleos set account permission bitsfleamain active '{"threshold": 1,"keys": [{"key": "EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW","weight": 1}],"accounts": [{"permission":{"actor":"bitsfleamain","permission":"eosio.code"},"weight":1}]}' owner -p bitsfleamain