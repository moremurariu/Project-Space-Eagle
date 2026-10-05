#!/bin/bash
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
run z165 prefix=$B cut=1208 beam=2000 maxt=95 mode=1 chord=1 H=600 capk=1150 firefrom=1165
run z165b prefix=$B cut=1208 beam=2000 maxt=95 mode=1 H=300 capk=1156 firefrom=1165
run z168 prefix=$B cut=1208 beam=2000 maxt=95 mode=1 chord=1 H=600 capk=1150 firefrom=1168
