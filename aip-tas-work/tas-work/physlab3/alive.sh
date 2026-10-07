#!/bin/bash
# alive.sh FILE...: does a continuation from the file's end reach x <= 7300 within 45 ticks without freeze? (hp beam 1500)
cd $(dirname $0)
for f in "$@"; do n=$(wc -l < $f); r=$(nice -n 5 ./hp ../AiP-Gores.map prefix=$f cut=$n beam=1500 maxt=45 mode=1 gatex=7300 gatey=1e9 gatey0=0 gatevx=5 survive=15 out=runs/_alive 2>&1 | grep -m1 GATE); echo "$(basename $f): ${r:-DOOMED?}"; done
