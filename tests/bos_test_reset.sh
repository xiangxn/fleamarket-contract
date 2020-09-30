export BOS_API_URL="https://api-bostest.blockzone.net"
export SYMBOL="BOS"
export USE_OLD_RPC="--use-old-rpc"
# deploy
cleos wallet unlock -n bostest  --password PW5KajM5kmn6YSckdg3hWregB1bGQ3VMKeUpSyHDMXuukZZnTiyFj
cleos -u $BOS_API_URL set contract bitsfleamain ./build/bitsfleamain -p bitsfleamain $USE_OLD_RPC &&
cleos -u $BOS_API_URL push action bitsfleamain reset '[]' -p bitsfleamain $USE_OLD_RPC

JSON=`cleos -u $BOS_API_URL get scope bitsfleamain -t accounts`
USER_NAME=`echo ${JSON} | grep -o '"scope": "[^"]*'`
for NAME in $USER_NAME
    do
        if [ $NAME != '"scope":' ]; then
            echo "process "${NAME:1}" ..."
            PAR=`echo "[\""${NAME:1}"\"]"`
            cleos -u $BOS_API_URL push action bitsfleamain cleanscope $PAR -p bitsfleamain $USE_OLD_RPC
        fi
    done