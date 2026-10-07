#!/usr/bin/env python3
"""Straighter reference line: Teero's track smoothed with a +-W point moving average (k labels kept), but only where
the smoothed point keeps >= clear px from solid / freeze tiles (else the largest smaller window that does).
usage: smoothref.py OUT W [clear=40] [k0=1000] [k1=99999]"""
import sys, math
out = sys.argv[1]; Wn = int(sys.argv[2]); clear = float(sys.argv[3]) if len(sys.argv) > 3 else 40
k0 = int(sys.argv[4]) if len(sys.argv) > 4 else 1000; k1 = int(sys.argv[5]) if len(sys.argv) > 5 else 99999
rows = open('map.txt').read().splitlines()
def bad(x, y):
    r = int(math.ceil(clear / 32)) + 1
    tx, ty = int(x // 32), int(y // 32)
    for j in range(ty - r, ty + r + 1):
        for i in range(tx - r, tx + r + 1):
            if j < 0 or j >= len(rows) or i < 0 or i >= len(rows[j]) or rows[j][i] in '#f':
                # distance from the point to the tile square
                dx = max(i * 32 - x, 0, x - (i * 32 + 32)); dy = max(j * 32 - y, 0, y - (j * 32 + 32))
                if math.hypot(dx, dy) < clear:
                    return True
    return False
P = []
for l in open('teero_track.txt'):
    a = l.split(); P.append((int(a[0]), float(a[1]), float(a[2])))
N = len(P); res = []; nsm = 0
for i, (k, x, y) in enumerate(P):
    if k < k0 or k > k1:
        res.append((k, x, y)); continue
    best = (x, y)
    w = Wn
    while w >= 1:
        lo, hi = max(0, i - w), min(N - 1, i + w)
        sx = sum(P[j][1] for j in range(lo, hi + 1)) / (hi - lo + 1); sy = sum(P[j][2] for j in range(lo, hi + 1)) / (hi - lo + 1)
        if not bad(sx, sy):
            best = (sx, sy); nsm += w == Wn; break
        w //= 2
    res.append((k, best[0], best[1]))
open(out, 'w').write(''.join(f'{k} {x:.1f} {y:.1f}\n' for k, x, y in res))
L0 = sum(math.dist(P[i][1:], P[i + 1][1:]) for i in range(N - 1) if k0 <= P[i][0] <= k1)
L1 = sum(math.dist(res[i][1:], res[i + 1][1:]) for i in range(N - 1) if k0 <= res[i][0] <= k1)
print(f'{out}: full window at {nsm} points, length {L0:.0f} -> {L1:.0f} px in k {k0}..{k1}')
