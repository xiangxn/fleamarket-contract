
cleos push action bitsfleamain bindaddr '[0,"player","8,ETH","0x23Ee6406969A9739E695C7b0C6b66daF47b8d782"]' -p player &&

cleos push action bitsfleamain issue '[ "bitsfleamain", "100.00000000 ETH", "memo" ]' -p bitsfleamain &&

cleos push action bitsfleamain issue '[ "player", "100.00000000 ETH", "memo" ]' -p bitsfleamain &&

cleos transfer player bitsfleamain "0.10000000 ETH" "w:" -p player -c bitsfleamain &&

cleos get currency balance bitsfleamain player ETH &&

cleos get table bitsfleamain bitsfleamain othersettle