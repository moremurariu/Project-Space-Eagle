#!/usr/bin/env python3
"""Render map.txt + tracks to PNG (pure python). usage: drawrun.py OUT.png x0 y0 x1 y1 scale track1[:rgb[:k0:k1]] ...
tracks: "k x y" files (k label); colours r,g,b; dots every 10 ticks get darker."""
import sys, zlib, struct
out = sys.argv[1]; x0, y0, x1, y1 = map(float, sys.argv[2:6]); sc = float(sys.argv[6])
rows = open('map.txt').read().splitlines()
W = int((x1 - x0) * sc); H = int((y1 - y0) * sc)
img = bytearray([255] * (W * H * 3))
col = {'#': (90, 90, 90), 'f': (120, 160, 255), 'E': (255, 200, 0), 'F': (255, 0, 255), 'S': (0, 200, 0)}
def put(px, py, c):
    if 0 <= px < W and 0 <= py < H:
        i = (py * W + px) * 3; img[i:i + 3] = bytes(c)
for py in range(H):
    wy = y0 + py / sc; ty = int(wy // 32)
    for px in range(W):
        wx = x0 + px / sc; tx = int(wx // 32)
        ch = rows[ty][tx] if 0 <= ty < len(rows) and 0 <= tx < len(rows[ty]) else '#'
        if ch in col: put(px, py, col[ch])
for spec in sys.argv[7:]:
    a = spec.split(':'); path = a[0]
    c = tuple(int(v) for v in a[1].split(',')) if len(a) > 1 else (255, 0, 0)
    k0 = int(a[2]) if len(a) > 2 else -10**9; k1 = int(a[3]) if len(a) > 3 else 10**9
    pts = []
    for l in open(path):
        p = l.split()
        if len(p) >= 3:
            k = int(float(p[0]))
            if k0 <= k <= k1: pts.append((k, float(p[1]), float(p[2])))
    for (ka, xa, ya), (kb, xb, yb) in zip(pts, pts[1:]):
        n = int(max(abs(xb - xa), abs(yb - ya)) * sc) + 1
        for i in range(n + 1):
            t = i / n
            put(int((xa + (xb - xa) * t - x0) * sc), int((ya + (yb - ya) * t - y0) * sc), c)
    for k, x, y in pts:
        if k % 25 == 0:
            for dx in range(-2, 3):
                for dy in range(-2, 3):
                    put(int((x - x0) * sc) + dx, int((y - y0) * sc) + dy, tuple(v // 2 for v in c))
raw = b''.join(b'\x00' + bytes(img[r * W * 3:(r + 1) * W * 3]) for r in range(H))
def chunk(t, d): return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
open(out, 'wb').write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', W, H, 8, 2, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(raw, 6)) + chunk(b'IEND', b''))
print(out, W, H)
