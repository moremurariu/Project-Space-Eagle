#!/bin/bash
# runH.sh NAME PREFIX [extra]: corridor 1 with the energy beam (pre tool, post-start mode) from a pre-start prefix
# result: race ticks to x > 8800, appended to runs/H.res; best path in runs/H_NAME0.txt
cd "$(dirname "$0")"
name=$1; pf=$2; shift 2
t0=$(date +%s)
../ddnet/build-sim/pre AiP-Gores.map prefix=$pf postbeam=20000 maxticks=330 threads=2 gatex=8800 gatelambda=0 lambda=0.09 vref=25 out=runs/H_$name top=1 "$@" > runs/H_$name.log 2>&1
r=$(grep -m1 "^CROSS 0" runs/H_$name.log | awk '{print $4}')
rt=$(python3 -c "import math,sys; r=float(sys.argv[1]); print(math.ceil(-r-1e-6))" ${r:-1} 2>/dev/null)
echo "$name pf=$pf [$*] => rank ${r:-NONE} rt ${rt} ($(( $(date +%s)-t0 ))s)" >> runs/H.res
