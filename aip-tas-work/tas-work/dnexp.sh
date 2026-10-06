#!/bin/bash
# dnexp.sh NAME "extra args" [seeds] : U-turn-1 window from c1 (k1099 -> k1249), base = pchain settings
cd "$(dirname "$0")"
N=$1; X=$2; SEEDS=${3:-"1 2"}
BASE="horizon=150 beam=6000 threads=4 quiet=1 survevery=2 survive=30 rothook=1 quant=1 ghost=3 hnow=1000 ghoste=0.02 sinks=1156,1285,1443,1793,2143,2465 kcredit=2 kready=10 ghostsink=1500 latpen=0.2 latdz=24"
for sd in $SEEDS; do
  o=$(../ddnet/build-sim/segf AiP-Gores.map prefix=${PREFIX:-runs/rh/g1/c1.txt} gate=${GATE:-1249} maxticks=${MAXT:-360} out=runs/dn/${N}_s${sd}_ $BASE jitter=1 seed=$sd $X 2>&1 | grep -E "GATE rt|NOGATE|retro shots" | tr "\n" " ")
  echo "$N s$sd: $o"
done
