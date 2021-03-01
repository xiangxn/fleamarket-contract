export BOS_API_URL="https://api.bossweden.org"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
export CONTRACT_ACCOUNT="bitsfleamain"
export CONTRACT_ACTIVE_KEY="EOS7iNc2CYEHTs1aLdC9jK3Ptw6pKUFy1yNH5bvgD5T1yPxJvHyLv"
# deploy
cleos wallet unlock -n bosmain --password PW5Juc1wGqUrzYoRVHmQLs6shHMTwekLJ4gaz7AA1MQ9aCiphT46T
cleos -u $BOS_API_URL set contract $CONTRACT_ACCOUNT ./build/bitsfleamain -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
CONTRACT_DATA="{\"threshold\": 1,\"keys\": [{\"key\": \"${CONTRACT_ACTIVE_KEY}\",\"weight\": 1}],\"accounts\": [{\"permission\":{\"actor\":\"${CONTRACT_ACCOUNT}\",\"permission\":\"eosio.code\"},\"weight\":1}]}" &&
cleos -u $BOS_API_URL set account permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT active "$CONTRACT_DATA" owner &&
cleos -u $BOS_API_URL push action $CONTRACT_ACCOUNT init '[]' -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action $CONTRACT_ACCOUNT reguser '[fleafaucet11,"水龙头","561bafdf09a24814380ba4abb164418a20ac314cbffbcabf2e444a11baff6258","753602941f5b21b558616332b23ea10abff9dd53",0,"EOS5upuA17p2Z8zBscLRaXQtSHuiaDfNRP8ZJ9PPHarN8sFPZYfSh"]' -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
cleos -u $BOS_API_URL get table $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT global

# 授权水龙头创建用户权限
cleos -u $BOS_API_URL set account permission fleafaucet11 creater 'EOS5upuA17p2Z8zBscLRaXQtSHuiaDfNRP8ZJ9PPHarN8sFPZYfSh' active -p fleafaucet11@active $USE_OLD_RPC
cleos -u $BOS_API_URL set action permission -p fleafaucet11@active $USE_OLD_RPC fleafaucet11 eosio newaccount creater
cleos -u $BOS_API_URL set action permission -p fleafaucet11@active $USE_OLD_RPC fleafaucet11 eosio buyrambytes creater 
cleos -u $BOS_API_URL set action permission -p fleafaucet11@active $USE_OLD_RPC fleafaucet11 eosio delegatebw creater 

#授权平台执行权限
cleos -u $BOS_API_URL set account permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT execute 'EOS6xf3J5cLnEzTvsXjSncremGV7s1CBk8S3ovLnimuLs9q2k5BKz' active 
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT reguser execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT publish execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT placeorder execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT closesettle execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT setpayaddr execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT startsync execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT endsync execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT updatecoin execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT deletelog execute
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT cleanorder execute

#授权平台分发收益权限
cleos -u $BOS_API_URL set account permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT allot 'EOS8Ce7ePDFh1tDEvxW1D59NeV1LHu4qPso52mP7Fz2oDpWAeNLTi' active 
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT eosio.token transfer allot
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT $CONTRACT_ACCOUNT transfer allot
cleos -u $BOS_API_URL set action permission -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT bosibc.io transfer allot

#给网关账号发行映射资产
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT issue '[ "fleagateways", "1000000.00000 BTS", "init" ]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT issue '[ "fleagateways", "1000000.00000000 NULS", "init" ]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT updatecoin '["5,BTS","1.00000 BTS",true,"1000000000.00000 BTS"]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT updatecoin '["8,HT","0.10000000 HT",true,"500000000.00000000 HT"]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT issue '[ "fleagateways", "1000000.00000000 HT", "init" ]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT setcfg '[1,0.03]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT updatecoin '["8,BNB","0.10000000 BNB",true,"200000000.00000000 BNB"]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT issue '[ "fleagateways", "1000000.00000000 BNB", "init" ]'

# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT updatecoin '["8,BUSD","1.00000000 BUSD",true,"1000000000.00000000 BUSD"]'
# cleos -u $BOS_API_URL push action -p $CONTRACT_ACCOUNT $USE_OLD_RPC $CONTRACT_ACCOUNT issue '[ "fleagateways", "1000000.00000000 BUSD", "init" ]'
# contract address: 0xe9e7cea3dedca5984780bafc599bd69add087d56

