#!/usr/bin/env python3
"""Lag of a run behind Teero's track: for each race tick, the track tick of the nearest track point (searched
forward along the track) -> lag = rt - k.  usage: lagtrack.py INPUTS [from_rt] [to_rt] [every] [-- extra lab cmds]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
T = {}
for l in open('teero_track.txt'):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
ks = sorted(T)
a = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 0; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 99999
ev = int(sys.argv[4]) if len(sys.argv) > 4 else 10
cmd = sys.argv[5] if len(sys.argv) > 5 else 'replay ' + a
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', cmd], capture_output=True, text=True).stdout
X = {}
for l in out.splitlines():
    m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)', l)
    if m and l.startswith('in ') and int(m.group(1)) not in X:
        X[int(m.group(1))] = tuple(float(m.group(i)) for i in range(3, 7))
kc = None
for rt in sorted(X):
    if rt < r0 or rt > r1: continue
    x, y, vx, vy = X[rt]
    lo = (kc - 30) if kc is not None else max(ks[0], rt - 150)
    best = min((k for k in range(lo, lo + 180) if k in T), key=lambda k: (T[k][0] - x) ** 2 + (T[k][1] - y) ** 2)
    kc = best
    if rt % ev == 0:
        d = math.hypot(T[best][0] - x, T[best][1] - y)
        print(f'rt {rt:4d} pos {x:6.0f} {y:5.0f} |v| {math.hypot(vx, vy):5.1f}  nearest k {best} (off {d:4.0f} px)  lag {rt - best:+d}')
