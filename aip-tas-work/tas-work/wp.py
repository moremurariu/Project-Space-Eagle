#!/usr/bin/env python3
"""wp.py TRACK [TRACK2...] -- k0 k1 step : for Teero waypoints every `step` Teero ticks in [k0,k1], the tick at which each
track (k x y files) passes closest to the waypoint (sequential search), and the lag vs Teero."""
import sys, math
a = sys.argv[1:]; i = a.index('--'); tracks = a[:i]; k0, k1, st = map(int, a[i+1:i+4])
def load(p):
    T = {}
    for l in open(p):
        s = l.split()
        if len(s) >= 3: T[int(float(s[0]))] = (float(s[1]), float(s[2]))
    return T
R = load('teero_track.txt'); Ts = [load(p) for p in tracks]
cur = [None] * len(Ts)
print('teero_k  ' + '  '.join('%-18s' % p.split('/')[-1][:18] for p in tracks))
for k in range(k0, k1 + 1, st):
    w = R[k]; row = []
    for j, T in enumerate(Ts):
        ks = sorted(T)
        lo = cur[j] if cur[j] is not None else ks[0]
        cand = [q for q in ks if lo - 5 <= q <= lo + 200] if cur[j] is not None else ks
        best = min(cand, key=lambda q: math.dist(T[q], w))
        cur[j] = best
        row.append('%5d %+4d (%3.0f)' % (best, k - best, math.dist(T[best], w)))
    print('%5d    ' % k + '  '.join(row))
