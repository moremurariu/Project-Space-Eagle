#!/usr/bin/env python3
"""Judge candidate cuts of runs that left the incumbent: from each cut, x_tig follows the incumbent (run2ref reference,
off = the cut's lead, shadow = incumbent) for J ticks; prints the lead (x_ds root progress) at the cut and at cut + J.
usage: judge.py INC J OUTDIR RUN:CUT[,CUT...] [RUN:CUT...] [seeds=1] [workers=4] [tigx="..."]"""
import os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
B = '../ddnet/build-sim/'
inc, J, D = sys.argv[1], int(sys.argv[2]), sys.argv[3]
os.makedirs(D, exist_ok=True)
kw = dict(a.split('=', 1) for a in sys.argv[4:] if '=' in a and ':' not in a.split('=')[0])
jobs = [a for a in sys.argv[4:] if ':' in a and '=' not in a.split(':')[0]]
NS, NW = int(kw.get('seeds', 1)), int(kw.get('workers', 4))
TIGX = kw.get('tigx', '').split()
TIG = ['beam=3000', 'every=1000', 'tolf=3', 'tolj=2', 'pendwin=6', 'lagw=0.3', 'pendb=4', 'freeb=2', 'threads=1']
def sh(a): return subprocess.run(a, capture_output=True, text=True).stdout
if not os.path.exists(f'{D}/inc.csv'):
    sh(['python3', 'run2ref.py', inc, f'{D}/inc_track.txt', f'{D}/inc.csv'])
def cut(run, rt, path):
    L = [l for l in open(run).read().splitlines() if l.strip()]
    open(path, 'w').write('\n'.join(L[:rt + 68]) + '\n'); return path
def prog(pre):
    m = re.search(r'root progress ([\d.]+)', sh([B + 'x_ds', 'AiP-Gores.map', f'inc={inc}', f'prefix={pre}', 'gate=finish', 'maxsteps=0', 'verbose=0']))
    return float(m.group(1)) if m else -1
def one(t):
    run, c, sd = t
    tag = f'{os.path.basename(run).replace(".txt", "")}_c{c}_s{sd}'
    pre = cut(run, c, f'{D}/{tag}_pre.txt')
    l0 = prog(pre) - c
    out = f'{D}/{tag}.txt'
    sh([B + 'x_tig', 'AiP-Gores.map', pre, f'csv={D}/inc.csv', f'track={D}/inc_track.txt', f'off={l0:.1f}', f'maxt={c + J}', f'out={out}',
        f'seed={sd}', f'jitter={0 if sd == 0 else 3}', 'shadow=' + inc] + TIG + TIGX)
    n = sum(1 for z in open(out) if z.strip()) if os.path.exists(out) else 0
    if n - 68 < c + J - 2:
        return (tag, l0, None, n - 68)
    e = c + J
    return (tag, l0, prog(cut(out, e, f'{D}/{tag}_e.txt')) - e, e)
T = []
for j in jobs:
    run, cs = j.rsplit(':', 1)
    for c in cs.split(','):
        for sd in range(NS):
            T.append((run, int(c), sd))
with ThreadPoolExecutor(NW) as ex:
    for tag, l0, l1, e in ex.map(one, T):
        print(f'{tag}: lead {l0:+.1f} at cut -> ' + (f'{l1:+.1f} at {e}' if l1 is not None else f'died at {e}'), flush=True)
