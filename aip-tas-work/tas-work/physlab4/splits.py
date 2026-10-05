#!/usr/bin/env python3
"""splits.py TRACE...: first tick at each gate of the final maze (Teero's in the header)."""
import sys, re
GATES = [  # name, test(x,y), sequential
    ('col y<4190', lambda x, y: y < 4190 and 8470 < x < 8580),
    ('corner y<3744', lambda x, y: y < 3744),
    ('top x>8900', lambda x, y: x > 8900 and y < 3750),
    ('right y>3900', lambda x, y: y > 3900 and x > 9100),
    ('under x<9050', lambda x, y: x < 9050 and y > 3870),
    ('hole y>4150', lambda x, y: y > 4150),
    ('gap y>4352', lambda x, y: y > 4352 and x > 9200),
    ('room x<9000', lambda x, y: x < 9000 and y > 4410),
    ('col x<8800', lambda x, y: x < 8800 and y > 4410),
    ('finish', None),
]
def splits(path):
    pts = []; fin = None
    for l in open(path):
        m = re.search(r'k (\d+) pos (-?\d+) (-?\d+).* fin (-?\d+)', l)
        if m:
            pts.append((int(m.group(1)), float(m.group(2)), float(m.group(3))))
            if int(m.group(4)) >= 0 and fin is None: fin = int(m.group(4))
    out = []; i = 0
    for name, f in GATES:
        if f is None:
            out.append(fin); continue
        t = None
        while i < len(pts):
            k, x, y = pts[i]
            if f(x, y): t = k; break
            i += 1
        out.append(t)
    return out
def teero():
    pts = []
    for l in open('../teero_track.txt'):
        p = l.split()
        if len(p) >= 3 and int(p[0]) >= 2385: pts.append((int(p[0]), float(p[1]), float(p[2])))
    out = []; i = 0
    for name, f in GATES:
        if f is None: out.append(2539); continue
        t = None
        while i < len(pts):
            k, x, y = pts[i]
            if f(x, y): t = k; break
            i += 1
        out.append(t)
    return out
print('%-28s' % 'gate' + ''.join('%14s' % g[0] for g in GATES))
print('%-28s' % 'Teero (track)' + ''.join('%14s' % v for v in teero()))
for p in sys.argv[1:]:
    print('%-28s' % p[-28:] + ''.join('%14s' % v for v in splits(p)))
