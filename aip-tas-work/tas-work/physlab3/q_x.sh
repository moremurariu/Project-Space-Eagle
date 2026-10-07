#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
for a in 16 20; do
run fx$a prefix=$B cut=1208 beam=1500 maxt=95 mode=1 chord=1 H=600 capk=1150 kg=1 kdecay=50 forcefire=1150:$a firefrom=1175
done
