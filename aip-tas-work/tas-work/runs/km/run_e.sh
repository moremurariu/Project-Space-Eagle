#!/bin/bash
# energy-ranked kicks: x_ds windows (incumbent tracking, high resolution), kick candidates ranked by speed gain
cd "$(dirname "$0")/../.."
V="seed=21 lam=0,0.004,0.01,0.02 kickmin=10 shadow=2 surv=12 trackfrac=0.3 shh=40 rollpre=4 shmode=1 kcred=0.5 kready=3 retro=4 egain=0.004 rotfar=1 brakew=0.3 beam=40000 cellpos=5 cellvel=0.6 threads=2 incforce=0 verbose=0"
E="kvalr=0.1 kvale=1 rrays=64 retro=6 firer=60 firekeep=8"
job() { # name inc cut gate tag extra
  ../ddnet/build-sim/x_ds AiP-Gores.map inc=$2 cut=$3 gate=rt$4 out=runs/km/$1_$5.txt $V $6 2>&1 | grep -E "GATE|NOGATE" | sed "s/^/$1 $5: /" >> runs/km/results_e.txt
}
job w1 kog_full_best.txt 1170 1250 E "$E" & job w3 kog_full_best.txt 2340 2440 E "$E" & wait
job u1 kog_full_2584.txt 1095 1290 E "$E" & job w4 kog_full_best.txt 1700 1790 E "$E" & wait
