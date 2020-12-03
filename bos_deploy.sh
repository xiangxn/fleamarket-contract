export BOS_API_URL="https://api.bossweden.org"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
export CONTRACT_ACCOUNT="bitsfleamain"
export CONTRACT_ACTIVE_KEY="EOS7iNc2CYEHTs1aLdC9jK3Ptw6pKUFy1yNH5bvgD5T1yPxJvHyLv"
# deploy
cleos wallet unlock -n bosmain &&
cleos -u $BOS_API_URL set contract $CONTRACT_ACCOUNT ./build/bitsfleamain -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
CONTRACT_DATA='{"threshold": 1,"keys": [{"key": "'${CONTRACT_ACTIVE_KEY}'","weight": 1}],"accounts": [{"permission":{"actor":"'${CONTRACT_ACCOUNT}'","permission":"eosio.code"},"weight":1}]}'
cleos -u $BOS_API_URL set account permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT active "$CONTRACT_DATA" owner &&
cleos -u $BOS_API_URL push action $CONTRACT_ACCOUNT init '[]' -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
cleos -u $BOS_API_URL get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT global