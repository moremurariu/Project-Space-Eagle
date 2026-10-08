#!/bin/bash
cd "$(dirname "$0")/../.."
V="seed=21 lam=0,0.004,0.01,0.02 kickmin=10 shadow=2 surv=12 trackfrac=0.3 shh=40 rollpre=4 shmode=1 kcred=0.5 kready=3 retro=4 egain=0.004 rotfar=1 brakew=0.3 beam=40000 cellpos=5 cellvel=0.6 threads=2 incforce=0 verbose=0"
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1249_orig.txt gate=rt1340 out=runs/kv/k1249_orig_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1249 orig: /" >> runs/kv/results.txt &
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1249_var.txt gate=rt1340 out=runs/kv/k1249_var_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1249 var: /" >> runs/kv/results.txt &
wait
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1962_orig.txt gate=rt2050 out=runs/kv/k1962_orig_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1962 orig: /" >> runs/kv/results.txt &
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1962_var.txt gate=rt2050 out=runs/kv/k1962_var_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1962 var: /" >> runs/kv/results.txt &
wait
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k2221_orig.txt gate=rt2330 out=runs/kv/k2221_orig_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k2221 orig: /" >> runs/kv/results.txt &
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k2221_var.txt gate=rt2330 out=runs/kv/k2221_var_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k2221 var: /" >> runs/kv/results.txt &
wait
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1581_orig.txt gate=rt1680 out=runs/kv/k1581_orig_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1581 orig: /" >> runs/kv/results.txt &
../ddnet/build-sim/x_ds AiP-Gores.map inc=kog_full_best.txt prefix=runs/kv/k1581_var.txt gate=rt1680 out=runs/kv/k1581_var_out.txt $V 2>&1 | grep -E "GATE|NOGATE|cut rt" | sed "s/^/k1581 var: /" >> runs/kv/results.txt &
wait
