#!/bin/bash
# kick-strength test: x_ds windows (incumbent tracking, high resolution) with kickmin 10 / 11 / 11.5
cd "$(dirname "$0")/../.."
V="seed=21 lam=0,0.004,0.01,0.02 shadow=2 surv=12 trackfrac=0.3 shh=40 rollpre=4 shmode=1 kcred=0.5 kready=3 retro=4 egain=0.004 rotfar=1 brakew=0.3 beam=40000 cellpos=5 cellvel=0.6 threads=2 incforce=0 verbose=0"
job() { # name cut gate kickmin
  ../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt cut=$2 gate=rt$3 out=runs/km/$1_k$4.txt $V kickmin=$4 2>&1 | grep -E "GATE|NOGATE" | sed "s/^/$1 k$4: /" >> runs/km/results.txt
}
for w in "w1 1170 1250" "w2 1835 1950" "w3 2340 2440" "w4 1700 1790"; do
  set -- $w
  job $1 $2 $3 10 & job $1 $2 $3 11 & wait
  job $1 $2 $3 11.5
done
