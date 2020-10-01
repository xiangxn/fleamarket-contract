export BOS_API_URL="https://api-bostest.blockzone.net"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
export CONTRACT_ACCOUNT="bitsfleatest"
# deploy
cleos wallet unlock -n bostest  --password PW5KajM5kmn6YSckdg3hWregB1bGQ3VMKeUpSyHDMXuukZZnTiyFj
cleos -u $BOS_API_URL set contract $CONTRACT_ACCOUNT ./build/bitsfleamain -p $CONTRACT_ACCOUNT $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action $CONTRACT_ACCOUNT reset '[]' -p $CONTRACT_ACCOUNT $USE_OLD_RPC

JSON=`cleos -u $BOS_API_URL get scope $CONTRACT_ACCOUNT -t accounts`
USER_NAME=`echo ${JSON} | grep -o '"scope": "[^"]*'`
for NAME in $USER_NAME
    do
        if [ $NAME != '"scope":' ]; then
            echo "process "${NAME:1}" ..."
            PAR=`echo "[\""${NAME:1}"\"]"`
            cleos -u $BOS_API_URL push action $CONTRACT_ACCOUNT cleanscope $PAR -p $CONTRACT_ACCOUNT $USE_OLD_RPC
        fi
    done