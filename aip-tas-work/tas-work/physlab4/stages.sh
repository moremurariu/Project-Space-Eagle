#!/bin/bash
# stages.sh NAME "ENTRY ARGS" "SEARCH ARGS": chained fbeam stages entry -> x>8900 -> under -> gap -> finish,
# each stage ranked at its gate by tick - subtick - gatevw * speed along the gate normal.
cd "$(dirname "$0")"
N=$1; E=$2; S=$3
VA=${VA:-0.5}; VB=${VB:-0.3}; VC=${VC:-0.3}
./fbeam ../AiP-Gores.map $E $S gate=x\>8900 gatevw=$VA gatewait=3 maxticks=90 out=runs/${N}_A > runs/${N}_A.log 2>&1 || exit 1
grep GATE runs/${N}_A.log
./fbeam ../AiP-Gores.map $E $S pre=runs/${N}_A.txt gate=under gatevw=$VB gatewait=3 maxticks=70 out=runs/${N}_B > runs/${N}_B.log 2>&1
grep GATE runs/${N}_B.log
./fbeam ../AiP-Gores.map $E $S pre=runs/${N}_B.txt gate=gap gatevw=$VC gatewait=3 maxticks=70 out=runs/${N}_C > runs/${N}_C.log 2>&1
grep GATE runs/${N}_C.log
./fbeam ../AiP-Gores.map $E $S pre=runs/${N}_C.txt gate=finish maxticks=80 out=runs/${N}_D > runs/${N}_D.log 2>&1
grep GATE runs/${N}_D.log
