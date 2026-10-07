#!/usr/bin/env python3
"""Run many randomized x_ds variants on one window in parallel; report and keep the best.
usage: dsmulti.py INC CUT GATE OUTDIR [n=24] [workers=4] [beam=1000] [extra x_ds args...]
The gate value is the crossing time minus egain x (energy - incumbent energy) (egain=0.004 by default)."""
import os, random, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
BIN = os.environ.get('DSBIN', '../ddnet/build-sim/x_ds')
inc, cut, gate, out = sys.argv[1:5]
kw = {}
extra = []
for a in sys.argv[5:]:
    k, _, v = a.partition('=')
    if k in ('n', 'workers', 'beam', 'egain', 'seed0'):
        kw[k] = v
    else:
        extra.append(a)
N = int(kw.get('n', 24))
W = int(kw.get('workers', 4))
BEAM = kw.get('beam', '1000')
EG = float(kw.get('egain', 0.004))
S0 = int(kw.get('seed0', 1))
os.makedirs(out, exist_ok=True)
LAMS = ['0,0.004,0.01,0.02', '0,0.004,0.01', '0,0.002,0.006,0.012', '0.002,0.006', '0,0.006,0.015']


def variant(i):
    r = random.Random(i * 7919 + S0)
    v = {
        'lam': r.choice(LAMS),
        'kickmin': r.choice(['0', '8', '10', '10', '11']),
        'quota': r.choice(['0', '20', '30', '50']),
        'rollh': r.choice(['0', '12', '16', '16', '24']),
        'credw': r.choice(['0', '0', '0.3', '0.6']),
        'credh': r.choice(['40', '60']),
        'jitter': r.choice(['0', '0.3', '0.6', '1.0']),
        'seed': str(i + S0),
        'surv': r.choice(['12', '16', '24']),
    }
    return v


def run(i):
    v = variant(i)
    tag = f'v{i + S0:03d}'
    args = [BIN, 'AiP-Gores.map', f'inc={inc}', f'cut={cut}', f'gate={gate}', 'threads=1', 'verbose=0', 'incforce=-1',
            f'beam={BEAM}', f'out={out}/{tag}.txt'] + [f'{k}={x}' for k, x in v.items()] + extra
    t0 = time.time()
    res = subprocess.run(args, capture_output=True, text=True).stdout
    m = re.search(r'GATE t ([\d.]+) \(incumbent ([\d.]+)\) E (-?\d+)', res)
    mv = re.search(r'verify \(CTasGame\): gate rt (-?\d+)', res)
    mi = re.search(r'incumbent reaches it at rt ([\d.]+) with E (-?\d+)', res)
    if not m:
        line = f'{tag} NOGATE ({time.time() - t0:.0f}s) ' + ' '.join(f'{k}={x}' for k, x in v.items())
        val = 1e9
    else:
        t, ti, e = float(m.group(1)), float(m.group(2)), float(m.group(3))
        ei = float(mi.group(2)) if mi else 0
        val = t - EG * (e - ei)
        line = (f'{tag} t {t:.2f} (inc {ti:.2f}) E {e:.0f} (inc {ei:.0f}) value {val:.2f} vrt {mv.group(1) if mv else "?"} '
                f'({time.time() - t0:.0f}s) ' + ' '.join(f'{k}={x}' for k, x in v.items()))
    with open(f'{out}/multi.log', 'a') as f:
        f.write(line + '\n')
    print(line, flush=True)
    return val, tag


with ThreadPoolExecutor(W) as ex:
    res = list(ex.map(run, range(N)))
res.sort()
print('BEST', res[0][1], f'{res[0][0]:.2f}')
