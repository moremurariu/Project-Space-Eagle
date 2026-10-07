#!/usr/bin/env python3
"""Taut reference line: Teero's track pulled tight (elastic band, k labels kept) while every point keeps >= clear px
from solid / freeze tiles. usage: tautref.py OUT clear [iters=1500] [k0=1000] [k1=99999] [alpha=0.5]"""
import sys, math, os, pickle
out = sys.argv[1]; clear = float(sys.argv[2]); iters = int(sys.argv[3]) if len(sys.argv) > 3 else 1500
k0 = int(sys.argv[4]) if len(sys.argv) > 4 else 1000; k1 = int(sys.argv[5]) if len(sys.argv) > 5 else 99999
alpha = float(sys.argv[6]) if len(sys.argv) > 6 else 0.5
rows = open('map.txt').read().splitlines(); H = len(rows); Wt = len(rows[0])
G = 8; gw, gh = Wt * 32 // G, H * 32 // G
cache = 'distfield.pkl'
if os.path.exists(cache):
    D = pickle.load(open(cache, 'rb'))
else:
    badt = [[rows[j][i] in '#f' for i in range(Wt)] for j in range(H)]
    D = [[0.0] * gw for _ in range(gh)]
    for cy in range(gh):
        y = cy * G + G / 2; ty = int(y // 32)
        for cx in range(gw):
            x = cx * G + G / 2; tx = int(x // 32)
            if badt[ty][tx]:
                continue
            best = 200.0
            for j in range(max(0, ty - 4), min(H, ty + 5)):
                for i in range(max(0, tx - 4), min(Wt, tx + 5)):
                    if badt[j][i]:
                        dx = max(i * 32 - x, 0, x - (i * 32 + 32)); dy = max(j * 32 - y, 0, y - (j * 32 + 32))
                        d = math.hypot(dx, dy)
                        if d < best: best = d
            D[cy][cx] = best
    pickle.dump(D, open(cache, 'wb'))
def dist(x, y):
    cx, cy = int(x // G), int(y // G)
    if cx < 0 or cy < 0 or cx >= gw or cy >= gh: return 0
    return D[cy][cx]
P = []
for l in open('teero_track.txt'):
    a = l.split(); P.append([int(a[0]), float(a[1]), float(a[2])])
idx = [i for i, p in enumerate(P) if k0 <= p[0] <= k1]
lo, hi = idx[0], idx[-1]
for it in range(iters):
    moved = 0
    for i in range(lo + 1, hi):
        ax, ay = (P[i - 1][1] + P[i + 1][1]) / 2, (P[i - 1][2] + P[i + 1][2]) / 2
        tx, ty = P[i + 1][1] - P[i - 1][1], P[i + 1][2] - P[i - 1][2]
        tl = math.hypot(tx, ty) or 1.0
        tx, ty = tx / tl, ty / tl
        mx, my = ax - P[i][1], ay - P[i][2]
        m = mx * -ty + my * tx  # move only along the normal (keeps the k labels' arc positions)
        nx, ny = P[i][1] + alpha * m * -ty, P[i][2] + alpha * m * tx
        if dist(nx, ny) >= clear or dist(nx, ny) >= dist(P[i][1], P[i][2]):
            P[i][1], P[i][2] = nx, ny; moved += 1
open(out, 'w').write(''.join(f'{k} {x:.1f} {y:.1f}\n' for k, x, y in P))
L = sum(math.dist(P[i][1:], P[i + 1][1:]) for i in range(lo, hi))
print(f'{out}: length {L:.0f} px (k {P[lo][0]}..{P[hi][0]}), min clearance {min(dist(P[i][1], P[i][2]) for i in range(lo, hi + 1)):.0f}')
