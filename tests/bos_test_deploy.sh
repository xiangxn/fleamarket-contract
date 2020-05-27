export BOS_API_URL="https://api-bostest.blockzone.net"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
# buy resources
#cleos -u $BOS_API_URL get currency balance eosio.token bitsfleamain BOS
#cleos -u $BOS_API_URL system buyram bitsfleamain bitsfleamain "2000 ${SYMBOL}" $USE_OLD_RPC
#cleos -u $BOS_API_URL system delegatebw bitsfleamain bitsfleamain "10 ${SYMBOL}" "400 ${SYMBOL}" $USE_OLD_RPC
# deploy
cleos wallet unlock -n bostest  --password PW5KajM5kmn6YSckdg3hWregB1bGQ3VMKeUpSyHDMXuukZZnTiyFj
cleos -u $BOS_API_URL set contract bitsfleamain ./build/bitsfleamain -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL set account permission bitsfleamain active '{"threshold": 1,"keys": [{"key": "EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW","weight": 1}],"accounts": [{"permission":{"actor":"bitsfleamain","permission":"eosio.code"},"weight":1}]}' owner -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain init '[]' -p bitsfleamain $USE_OLD_RPC &&
# reg user
cleos -u $BOS_API_URL push action bitsfleamain reguser '[reviewer1111,"reviewer1111","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B492",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain reguser '[reviewer1112,"reviewer1112","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B493",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain reguser '[reviewer1113,"reviewer1113","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B494","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B494",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain reguser '[bitsflea1111,"bitsflea1111","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B495","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B495",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain reguser '[bitsflea1112,"bitsflea1112","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B496","05FBED483A974456708EF12FCF3D278276E982234D775ECEE0DFA5CB49B8B496",0,"EOS5BiYrPwXwFmrjLQ3ZUa3BX9crdomJNfYdu6uC863XAXrHNyWbo"]' -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain appreviewer '[0,"reviewer1111"]' -p reviewer1111 $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain appreviewer '[1,"reviewer1112"]' -p reviewer1112 $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain appreviewer '[2,"reviewer1113"]' -p reviewer1113 $USE_OLD_RPC &&
# vote reviewer
cleos -u $BOS_API_URL push action bitsfleamain votereviewer '[3,"bitsflea1111",0,true]' -p bitsflea1111 $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain votereviewer '[3,"bitsflea1111",1,true]' -p bitsflea1111 $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain votereviewer '[3,"bitsflea1111",2,true]' -p bitsflea1111 $USE_OLD_RPC &&
cleos -u $BOS_API_URL get table bitsfleamain bitsfleamain global