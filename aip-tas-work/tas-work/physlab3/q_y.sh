#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
for m in 1144 1146 1142; do for y in 1960 1990; do
run y${m}_$y prefix=$B cut=1192 beam=1500 maxt=110 mode=6 chord=1 H=600 capk=1150 forcetgt=$m:9504,$y firefrom=$((m+25)) out=runs/y${m}_$y
done; done
