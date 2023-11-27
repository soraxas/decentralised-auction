#!/bin/sh

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

python sub.py alex &
python sub.py bob &
python sub.py charlie &
python sub.py oscar &

wait
