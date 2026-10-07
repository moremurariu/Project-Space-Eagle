#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
run m2x prefix=runs/pX30.txt cut=1218 beam=2000 maxt=90 mode=2
run m2b prefix=$B cut=1192 beam=2000 maxt=110 mode=2
