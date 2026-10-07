#!/usr/bin/env python3
"""Teero's velocity from his track: local quadratic fit of x(k), y(k) over +-W ticks -> displacement per tick
(derivative at k-0.5 = the move into tick k), then de-ramp x: dx = vx * ramp(|v|).
usage: teerovel.py K0 K1 [W]"""
import math, sys
tr = {}
for l in open('../teero_track.txt'):
    p = l.split()
    if len(p) >= 3:
        tr[int(p[0])] = (float(p[1]), float(p[2]))
def ramp(v):
    return 1.4 ** (-(50 * v - 550) / 2000) if v > 11 else 1.0
def deramp(dx, vy):
    lo, hi = 0.0, 300.0
    s = 1 if dx >= 0 else -1
    dx = abs(dx)
    for _ in range(60):
        m = (lo + hi) / 2
        if m * ramp(math.hypot(m, vy)) < dx: lo = m
        else: hi = m
    return s * lo
def solve3(A, b):
    # gaussian elimination 3x3
    M = [A[i][:] + [b[i]] for i in range(3)]
    for c in range(3):
        p = max(range(c, 3), key=lambda r: abs(M[r][c]))
        M[c], M[p] = M[p], M[c]
        for r in range(3):
            if r != c:
                f = M[r][c] / M[c][c]
                for cc in range(c, 4): M[r][cc] -= f * M[c][cc]
    return [M[i][3] / M[i][i] for i in range(3)]
def qfit(ts, vs):
    S = [[sum(t ** (i + j) for t in ts) for j in range(3)] for i in range(3)]
    b = [sum(v * t ** i for t, v in zip(ts, vs)) for i in range(3)]
    return solve3(S, b)  # c0 + c1 t + c2 t^2
def fit(k, W):
    ks = [j for j in range(k - W, k + W + 1) if j in tr]
    ts = [j - k for j in ks]
    cx = qfit(ts, [tr[j][0] for j in ks]); cy = qfit(ts, [tr[j][1] for j in ks])
    dx = cx[1] - cx[2]; dy = cy[1] - cy[2]  # derivative at t=-0.5
    return cx[0], cy[0], dx, dy, 2 * cx[2], 2 * cy[2]
if __name__ == '__main__':
    W = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    for k in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        x, y, dx, dy, ax, ay = fit(k, W)
        vx = deramp(dx, dy)
        print(f"k {k} pos {x:7.1f} {y:7.1f} disp {dx:6.2f} {dy:6.2f} acc {ax:5.2f} {ay:5.2f} -> v {vx:6.2f} {dy:6.2f} |v| {math.hypot(vx,dy):5.1f}")
