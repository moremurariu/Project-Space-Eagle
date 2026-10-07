#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
for T in 1168 1169 1170 1167; do
run k6_$T prefix=$B cut=1208 beam=2000 maxt=95 mode=6 forcetgt=$T:9350,2144 rvoff=-38,-21 rvv=-25,10 firefrom=$((T+25))
done
