#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
run k5v1 prefix=$B cut=1208 beam=2000 maxt=95 mode=6 forcetgt=1150:9472,1840 rvoff=-30,55 rvv=3,17 firefrom=1175
run k5v2 prefix=$B cut=1208 beam=2000 maxt=95 mode=6 forcetgt=1150:9472,1855 rvoff=-25,50 rvv=2,17 firefrom=1175
run k5v3 prefix=$B cut=1208 beam=2000 maxt=95 mode=6 forcetgt=1151:9472,1850 rvoff=-28,52 rvv=2,17 firefrom=1176
