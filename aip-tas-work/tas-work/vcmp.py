#!/usr/bin/env python3
"""Speed vs Teero at the same place: per race tick of a run, nearest Teero track point (searched forward), lag,
our |v| and Teero's |v| there (track displacement over +-2 ticks, velocity ramp inverted).
usage: vcmp.py INPUTS from_rt to_rt [every]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
T = {}
for l in open('teero_track.txt'):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
ks = sorted(k for k in T if k >= 0)
def disp(v): return v * 1.4 ** (-(50 * v - 550) / 2000) if v > 11 else v
def inv(d):
    lo, hi = 0.0, 200.0
    for _ in range(50):
        m = (lo + hi) / 2
        if disp(m) < d: lo = m
        else: hi = m
    return lo
def tv(k):
    a, b = T.get(k - 2), T.get(k + 2)
    if not a or not b: return 0
    return inv(math.dist(a, b) / 4)
a = sys.argv[1]; r0 = int(sys.argv[2]); r1 = int(sys.argv[3]); ev = int(sys.argv[4]) if len(sys.argv) > 4 else 10
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + a], capture_output=True, text=True).stdout
X = {}
for l in out.splitlines():
    m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)', l)
    if m and l.startswith('in ') and int(m.group(1)) not in X:
        X[int(m.group(1))] = tuple(float(m.group(i)) for i in range(3, 7))
kc = None
for rt in sorted(X):
    x, y, vx, vy = X[rt]
    if kc is None:
        kc = min(ks, key=lambda k: math.dist(T[k], (x, y)))
    else:
        cand = [k for k in ks if kc - 5 <= k <= kc + 40]
        kc = min(cand, key=lambda k: math.dist(T[k], (x, y)))
    if r0 <= rt <= r1 and rt % ev == 0:
        v = math.hypot(vx, vy); t = tv(kc)
        print(f'rt {rt} pos {x:6.0f} {y:5.0f}  k {kc} lead {kc - 3 - rt:+d}  |v| {v:5.1f}  teero {t:5.1f}  dE {v*v - t*t:+6.0f}')
