#!/usr/bin/env python3
"""Compare a run's x(race tick) with Teero's measured x (teero/hpos_first7s.csv, aligned so that our race tick
rt = (video_s - T0) * 50).  usage: teerox.py INPUTS [every] [T0=1.4467]"""
import re, subprocess, sys, os
HERE = os.path.dirname(os.path.abspath(__file__))
T0 = float(sys.argv[3]) if len(sys.argv) > 3 else 1.4467
ev = int(sys.argv[2]) if len(sys.argv) > 2 else 10
V = []
for l in open(os.path.join(HERE, 'teero/hpos_first7s.csv')):
    if l.startswith('#'): continue
    t, a, b, g = l.strip().split(',')
    if a != 'nan': V.append((float(t), float(a), float(b)))
def tx(rt, col=1):
    t = T0 + rt / 50
    for p, q in zip(V, V[1:]):
        if p[0] <= t <= q[0]:
            f = (t - p[0]) / (q[0] - p[0]); return p[col] + f * (q[col] - p[col])
    return None
out = subprocess.run([os.path.join(HERE, '../ddnet/build-sim/lab'), 'AiP-Gores.map', 'replay ' + sys.argv[1]], capture_output=True, text=True).stdout
X = {}
for l in out.splitlines():
    m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+)', l)
    if m and l.startswith('in '): X[int(m.group(1))] = (float(m.group(3)), float(m.group(4)))
g = next((r for r in sorted(X) if r >= 0 and X[r][0] > 8800), None)
print(f'x>8800 at rt {g}')
for rt in range(0, 280, ev):
    t = tx(rt)
    if rt in X and t is not None:
        print(f'rt {rt:3d} ours x {X[rt][0]:6.0f} y {X[rt][1]:5.0f}  teero x {t:6.0f}  lead {X[rt][0]-t:+5.0f} px')
