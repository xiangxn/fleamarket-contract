export BOS_API_URL="https://api-bostest.blockzone.net"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
export CONTRACT_ACCOUNT="bitsfleamain"
export CONTRACT_ACTIVE_KEY="EOS6VqjyRiJZWUDFErcnX7CkhDkkBKjaZCsLfGJXPJEEe5DNbW2KW"

cleos wallet unlock -n bostest  --password PW5KajM5kmn6YSckdg3hWregB1bGQ3VMKeUpSyHDMXuukZZnTiyFj

cleos -u $BOS_API_URL set contract $CONTRACT_ACCOUNT ./clear -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&


cleos -u $BOS_API_URL system sellram $CONTRACT_ACCOUNT 2998000 $USE_OLD_RPC


echo "done..."