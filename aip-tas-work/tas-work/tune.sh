#!/bin/bash
# tune.sh NAME [seg args...]: window Teero 401 -> 551 from the chain commit c4; result line in runs/tune/tune.res
cd "$(dirname "$0")"
n=$1; shift
t0=$(date +%s)
r=$(../ddnet/build-sim/seg AiP-Gores.map prefix=runs/rh/a/c4.txt gate=551 horizon=150 beam=20000 threads=1 maxticks=400 quiet=1 out=runs/tune/$n "$@" | grep -E "GATE|NOGATE")
echo "$n [$*] => $r ($(( $(date +%s)-t0 ))s)" >> runs/tune/tune.res
