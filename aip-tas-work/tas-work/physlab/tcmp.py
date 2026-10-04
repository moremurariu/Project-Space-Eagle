#!/usr/bin/env python3
"""Teero vs our run, matched by nearest point: Teero's smoothed (de-ramped) velocity and E vs ours.
usage: tcmp.py RUN k0 k1 step"""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from labx import *
T = {}
for l in open(os.path.join(WORK, 'teero_track.txt')):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
def ramp(v):
    return 1.4 ** (-(50 * v - 550) / 2000) if v > 11 else 1.0
def tstate(k, w=3):
    ks = range(k - w, k + w + 1); n = len(ks)
    mx = sum(T[i][0] for i in ks) / n; my = sum(T[i][1] for i in ks) / n
    den = sum((i - k) ** 2 for i in ks)
    dx = sum((i - k) * (T[i][0] - mx) for i in ks) / den; dy = sum((i - k) * (T[i][1] - my) for i in ks) / den
    v = math.hypot(dx, dy)
    for _ in range(40):
        vv = math.hypot(dx / ramp(v), dy); v += (vv - v) * 0.5
    return mx, my, dx / ramp(v), dy, dx
run_f = sys.argv[1]; k0, k1, st = map(int, sys.argv[2:5])
open(os.path.join(TMP, 'empty.txt'), 'w').close()
S = run(os.path.join(TMP, 'empty.txt'), read_inputs(run_f))
O = [s for s in S if s['rt'] >= 0]
for k in range(k0, k1 + 1, st):
    tx, ty, tvx, tvy, tdx = tstate(k)
    te = tvx ** 2 + tvy ** 2 - ty
    s = min(O, key=lambda s: (s['x'] - tx) ** 2 + (s['y'] - ty) ** 2 + (0 if abs(s['rt'] - k) < 40 else 1e9))
    i = O.index(s); dxo = O[i + 1]['x'] - s['x'] if i + 1 < len(O) else 0
    print(f"k{k} T({tx:6.0f},{ty:5.0f}) v({tvx:5.1f},{tvy:5.1f}) disp {tdx:5.1f} E {te:6.0f} | rt {s['rt']} lag {s['rt']-k+3:+d} ({s['x']:6.0f},{s['y']:5.0f}) v({s['vx']:5.1f},{s['vy']:5.1f}) disp {dxo:3.0f} E {E(s):6.0f} dE {E(s)-te:+5.0f} h{s['hook']} j{s['jumped']} g{s['gr']}")
