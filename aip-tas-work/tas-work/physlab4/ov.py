#!/usr/bin/env python3
"""ov.py TRACE [x0 x1 y0 y1]: map overlay (tiles) of a fbeam/fl trace ('*', digits = tick mod 10 every 5 ticks)
and Teero's track ('o'). Also prints the lag vs Teero at a few gates."""
import sys, re
lines = open('../map.txt').read().split('\n')
g = [list(l) for l in lines]
X0, X1, Y0, Y1 = 236, 300, 104, 148
if len(sys.argv) > 5:
    X0, X1, Y0, Y1 = map(int, sys.argv[2:6])
tr = {}
for l in open('../teero_track.txt'):
    p = l.split()
    if len(p) >= 3: tr[int(p[0])] = (float(p[1]), float(p[2]))
for k, (x, y) in tr.items():
    if 2370 <= k <= 2539:
        tx, ty = int(x // 32), int(y // 32)
        if g[ty][tx] == '.': g[ty][tx] = 'o'
ours = []
for l in open(sys.argv[1]):
    m = re.search(r'k (\d+) pos (-?\d+) (-?\d+)', l)
    if m: ours.append((int(m.group(1)), float(m.group(2)), float(m.group(3))))
for k, x, y in ours:
    tx, ty = int(x // 32), int(y // 32)
    c = g[ty][tx]
    g[ty][tx] = '*' if c in '.o*' else '!'
print('     ' + ''.join(str((x // 10) % 10) for x in range(X0, X1)))
print('     ' + ''.join(str(x % 10) for x in range(X0, X1)))
for y in range(Y0, Y1):
    print('%3d  ' % y + ''.join(g[y][X0:X1]))
