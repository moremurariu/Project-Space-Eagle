#!/usr/bin/env python3
"""Draw a run trace (x_trace output) on the map: path coloured by speed, explosions (circle = 48 px full-kick radius,
line to the tee; red = reduced force), fire ticks, labels every 25 race ticks.
usage: viz.py TRACE OUT.png [rt0 rt1] [x0 y0 x1 y1] [scale]"""
import os, sys
from PIL import Image, ImageDraw

tr, out = sys.argv[1], sys.argv[2]
rt0 = int(sys.argv[3]) if len(sys.argv) > 3 else -10**9
rt1 = int(sys.argv[4]) if len(sys.argv) > 4 else 10**9
T, E = [], []
for l in open(tr):
    a = l.split()
    if a[0] == 'T':
        rt = int(a[2])
        if rt0 <= rt <= rt1:
            T.append((rt, float(a[3]), float(a[4]), float(a[7]), int(a[9]), int(a[16])))
    elif a[0] == 'E':
        rt = int(a[2])
        if rt0 <= rt <= rt1:
            E.append((rt, float(a[4]), float(a[5]), float(a[7]), float(a[8]), float(a[10]), float(a[a.index('|f|') + 1])))
if len(sys.argv) > 8:
    x0, y0, x1, y1 = map(float, sys.argv[5:9])
else:
    xs = [t[1] for t in T]; ys = [t[2] for t in T]
    x0, y0, x1, y1 = min(xs) - 200, min(ys) - 200, max(xs) + 200, max(ys) + 200
sc = float(sys.argv[9]) if len(sys.argv) > 9 else 0.25
rows = open('map.txt').read().splitlines()
W, H = int((x1 - x0) * sc), int((y1 - y0) * sc)
img = Image.new('RGB', (W, H), (255, 255, 255))
d = ImageDraw.Draw(img)
col = {'#': (110, 110, 110), 'f': (150, 190, 255), 'E': (255, 200, 0), 'F': (255, 0, 255), 'S': (0, 200, 0)}
for ty in range(int(y0 // 32), int(y1 // 32) + 1):
    for tx in range(int(x0 // 32), int(x1 // 32) + 1):
        ch = rows[ty][tx] if 0 <= ty < len(rows) and 0 <= tx < len(rows[ty]) else '#'
        if ch in col:
            d.rectangle([(tx * 32 - x0) * sc, (ty * 32 - y0) * sc, ((tx + 1) * 32 - x0) * sc - 1, ((ty + 1) * 32 - y0) * sc - 1], fill=col[ch])
def P(x, y): return ((x - x0) * sc, (y - y0) * sc)
def vcol(v):
    t = max(0.0, min(1.0, (v - 10) / 55))
    return (int(255 * t), int(160 * (1 - abs(t - 0.5) * 2)), int(255 * (1 - t)))
for a, b in zip(T, T[1:]):
    d.line([P(a[1], a[2]), P(b[1], b[2])], fill=vcol(b[3]), width=2)
for t in T:
    if t[4] in (3, 4, 5):
        pass
    if t[0] % 25 == 0:
        x, y = P(t[1], t[2])
        d.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(0, 0, 0))
        d.text((x + 3, y - 10), str(t[0]), fill=(0, 0, 0))
for e in E:
    ex, ey = P(e[1], e[2]); tx, ty = P(e[3], e[4])
    c = (0, 160, 0) if e[6] > 11.9 else (220, 0, 0)
    r = 48 * sc
    d.ellipse([ex - r, ey - r, ex + r, ey + r], outline=c)
    d.line([(ex, ey), (tx, ty)], fill=c, width=1)
    d.text((ex + 3, ey + 2), '%d:%.0f' % (e[0], e[6]), fill=c)
# optional reference track overlay: TREF=track_file:label0:label1 (black dots, label every 25)
if os.environ.get('TREF'):
    f, l0, l1 = os.environ['TREF'].split(':')
    R = {}
    for l in open(f):
        a = l.split()
        if len(a) >= 3 and int(l0) <= int(a[0]) <= int(l1):
            R[int(a[0])] = (float(a[1]), float(a[2]))
    ks = sorted(R)
    for a, b in zip(ks, ks[1:]):
        d.line([P(*R[a]), P(*R[b])], fill=(0, 0, 0), width=1)
    for k in ks:
        if k % 25 == 0:
            x, y = P(*R[k])
            d.rectangle([x - 2, y - 2, x + 2, y + 2], fill=(0, 0, 0))
            d.text((x + 3, y + 3), 'T%d' % k, fill=(90, 0, 120))
img.save(out)
print(out, W, H)
