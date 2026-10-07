#!/bin/bash
# runT2.sh NAME PREFIX [extra]: corridor 1 including the turn (gate: x > 9312 and y > 450, i.e. falling into the
# right shaft) with the time model; result appended to runs/turn/T.res, best path runs/turn/NAME0.txt
cd "$(dirname "$0")"
name=$1; pf=$2; shift 2
t0=$(date +%s)
../ddnet/build-sim/pre AiP-Gores.map prefix=$pf postbeam=30000 maxticks=400 threads=2 gatex=9312 gatey=450 gatelambda=0 trref=teero_track.txt hnow=300 rothook=1 hookla=10 out=runs/turn/$name top=1 "$@" > runs/turn/$name.log 2>&1
r=$(grep -m1 "^CROSS 0" runs/turn/$name.log | awk '{print $4}')
echo "$name pf=$pf [$*] => rank ${r:-NONE} ($(( $(date +%s)-t0 ))s)" >> runs/turn/T.res
