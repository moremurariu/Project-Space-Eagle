#!/bin/bash
# lns.sh BASE.txt NAME "ENTRY ARGS" "SEARCH ARGS" C0 C1 STEP: re-search to the finish from cut points of BASE
# (post-entry inputs) with fbeam; prints the finish tick per cut, keeps runs/NAME_cC.txt
cd "$(dirname "$0")"
B=$1; N=$2; E=$3; S=$4; C0=$5; C1=$6; ST=$7
for ((c=C0; c<=C1; c+=ST)); do
  head -$c $B > runs/${N}_pre$c.txt
  ./fbeam ../AiP-Gores.map $E $S pre=runs/${N}_pre$c.txt gate=finish maxticks=200 out=runs/${N}_c$c > runs/${N}_c$c.log 2>&1
  echo "cut $c: $(grep -E 'GATE|NOGATE' runs/${N}_c$c.log | cut -c1-60)"
done
