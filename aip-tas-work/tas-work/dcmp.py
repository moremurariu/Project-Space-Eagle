#!/usr/bin/env python3
"""Displacement speed vs Teero at the same place (10-tick averages, no ramp inversion).
usage: dcmp.py TRACK(own k x y) from_rt to_rt [every]"""
import sys, math
def load(p):
    T = {}
    for l in open(p):
        a = l.split()
        if len(a) >= 3: T[int(float(a[0]))] = (float(a[1]), float(a[2]))
    return T
T = load('teero_track.txt'); O = load(sys.argv[1])
r0, r1 = int(sys.argv[2]), int(sys.argv[3]); ev = int(sys.argv[4]) if len(sys.argv) > 4 else 10
ks = sorted(k for k in T if k >= 0)
def d10(D, k):
    if k - 5 in D and k + 5 in D: return math.dist(D[k - 5], D[k + 5]) / 10
    return 0
kc = None
for k in sorted(O):
    rt = k - 3
    if rt < r0 - 50: continue
    p = O[k]
    if kc is None: kc = min(ks, key=lambda q: math.dist(T[q], p))
    else: kc = min([q for q in ks if kc - 5 <= q <= kc + 40], key=lambda q: math.dist(T[q], p))
    if r0 <= rt <= r1 and rt % ev == 0:
        print(f'rt {rt} pos {p[0]:6.0f} {p[1]:5.0f} k {kc} lead {kc - k:+d}  disp ours {d10(O, k):5.1f} teero {d10(T, kc):5.1f}')
