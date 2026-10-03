#!/usr/bin/env python3
"""Nudge a hook-only climb (dir held -1/0/+1 for a few ticks somewhere before the gap) and run pf on every nudged
climb; report the best exit stacks.

usage: pfperturb.py CLIMB OUTDIR [lo=10 hi=36 maxlen=6]   (offsets relative to the grenade pickup)
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
PF = '../ddnet/build-sim/pf'
MAP = 'AiP-Gores.map'
args = dict(a.split('=', 1) for a in sys.argv[3:])
LO, HI, MAXLEN = int(args.get('lo', 10)), int(args.get('hi', 36)), int(args.get('maxlen', 6))
base = open(sys.argv[1]).read().splitlines()
D = sys.argv[2]
os.makedirs(D, exist_ok=True)


def pf(path, out, top=2):
    res = subprocess.run([PF, MAP, f'base={path}', 'rel=1', 't1lo=3', 't1hi=40', 'tend=0', 'xf=5900', 'res=0.05', 'res2=1', 'keep=600',
                          f'top={top}', 'threads=1', f'out={out}'], capture_output=True, text=True).stdout
    pick = re.search(r'pickup after input (-?\d+)', res)
    m = re.search(r'PF 0 score ([-\d.]+) (.*) -> (\S+)', res)
    return (int(pick.group(1)) if pick else None), (float(m.group(1)) if m else None), (m.group(2) if m else ''), (m.group(3) if m else None)


pick, s0, d0, _ = pf(sys.argv[1], f'{D}/base_')
print(f'base: pickup {pick} score {s0} {d0}', flush=True)
results = []
n = 0
for d in (-1, 1, 0):
    for s in range(LO, HI, 2):
        for L in range(1, MAXLEN + 1, 2):
            lines = list(base)
            for k in range(pick + s, min(pick + s + L, len(lines))):
                f = lines[k].split()
                f[0] = str(d)
                lines[k] = ' '.join(f)
            path = f'{D}/c_{d}_{s}_{L}.txt'
            with open(path, 'w') as fh:
                fh.write('\n'.join(lines) + '\n')
            _, sc, desc, outp = pf(path, f'{D}/p_{d}_{s}_{L}_', top=1)
            n += 1
            if sc is not None:
                results.append((sc, d, s, L, desc, outp))
            if n % 10 == 0:
                best = min(results) if results else None
                print(f'{n} done, best {best[:4] if best else None}', flush=True)
results.sort()
for r in results[:8]:
    print('BEST', r[0], f'dir {r[1]} from +{r[2]} for {r[3]}', r[4][:90], '->', r[5], flush=True)
