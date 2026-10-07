#!/bin/bash
# sequential hp runs (one CPU-heavy process at a time)
cd $(dirname $0)
B=../runs/ex/e1025v1_0.txt
run() { name=$1; shift; (time nice -n 5 ./hp ../AiP-Gores.map prefix=$B out=runs/$name "$@") > runs/$name.log 2>&1; echo "$name $(grep GATE runs/$name.log | head -1)" >> runs/q.res; }
run m1a cut=1192 beam=2000 maxt=110 mode=1 H=300 capk=1156
run m1b cut=1192 beam=2000 maxt=110 mode=1 H=600 capk=1150
run m1c cut=1192 beam=2000 maxt=110 mode=1 H=600 capk=1150 kg=1.5 kdecay=15
run m1d cut=1192 beam=2000 maxt=110 mode=1 H=600 capk=1150 firefrom=1138 fireto=1150 fire2=1160
