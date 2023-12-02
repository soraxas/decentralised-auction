#!/bin/sh

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

echo "" >> decentralised_bidding/decentralised_bidding.cpp

#mosquitto 2>&1 >/dev/null &

python sub.py alex &
python sub.py bob &
python sub.py charlie &
python sub.py oscar &
python sub.py oscar1 &
python sub.py oscar2 &
python sub.py oscar3 &
python sub.py oscar4 &
python sub.py oscar5 &

wait
