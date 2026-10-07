#!/usr/bin/env python3
"""kaudit.py RDV_OWN_OUTPUT: classify each explosion of a run: the solid tile it hit is part of a freeze-lined wall
('wall') or a small free-standing block ('block', no freeze tile within 1 tile); kick strength and direction."""
import re, sys
M = [l.rstrip('\n') for l in open('map.txt')]
def t(x, y):
    return M[y][x] if 0 <= y < len(M) and 0 <= x < len(M[y]) else '#'
rows = []
for l in open(sys.argv[1]):
    m = re.search(r'expl rt (\d+) at ([\d.]+) ([\d.]+): tee ([\d.]+) ([\d.]+) dist (\d+) kick ([\d.]+) px/t cos ([-\d.]+) \|v\| ([\d.]+) -> ([\d.]+)', l)
    if not m: continue
    rt, ex, ey, tx, ty, d, k, c, v0, v1 = m.groups()
    ex, ey = float(ex), float(ey)
    # solid tile at/near the explosion point
    cx, cy = int(ex // 32), int(ey // 32)
    best = None
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            if t(cx + dx, cy + dy) == '#':
                best = (cx + dx, cy + dy); break
        if best: break
    kind = '?'
    if best:
        bx, by = best
        fr = sum(t(bx + i, by + j) == 'f' for i in range(-2, 3) for j in range(-2, 3))
        kind = 'wall' if fr > 0 else 'block'
    rows.append((int(rt), kind, int(d), float(k), float(c), float(v0), float(v1)))
for r in rows:
    print(f"rt {r[0]:4d} {r[1]:5s} dist {r[2]:3d} kick {r[3]:4.1f} cos {r[4]:5.2f} |v| {r[5]:5.1f} -> {r[6]:5.1f}")
import collections
c = collections.Counter(r[1] for r in rows)
for kind in c:
    rr = [r for r in rows if r[1] == kind]
    print(f"{kind}: {len(rr)} kicks, mean kick {sum(r[3] for r in rr)/len(rr):.2f}, full(>=11.9) {sum(r[3] >= 11.9 for r in rr)}, mean cos {sum(r[4] for r in rr)/len(rr):.2f}, mean d|v| {sum(r[6]-r[5] for r in rr)/len(rr):.2f}")
