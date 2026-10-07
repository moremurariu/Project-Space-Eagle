#!/bin/bash
# sweep_rv.sh PF... : run xrv on each prefix with a few parameter sets, evaluate
cd $(dirname $0)
for pf in "$@"; do
  b=$(basename $pf .txt)
  for prm in "dx=40 vxw=0" "dx=37 vxw=0" "dx=37 vxw=1" "dx=34 vxw=1" "dx=37 vxw=2 vxticks=3"; do
    tag=$(echo $prm | tr -d ' =')
    nice ./xrv ../AiP-Gores.map prefix=$pf beam=3000 $prm out=runs/rv/${b}_$tag.txt > /dev/null 2>&1
    [ -f runs/rv/${b}_$tag.txt ] && python3 ev.py runs/rv/${b}_$tag.txt
  done
done
