#!/usr/bin/env python3
"""cmp.py TRACE K0 K1: our state vs Teero's (smoothed velocity) tick by tick."""
import sys, re, math
sys.argv += []
import importlib.util
spec = importlib.util.spec_from_file_location('tv', 'teerovel.py'); tv = importlib.util.module_from_spec(spec); spec.loader.exec_module(tv)
ours = {}
for l in open(sys.argv[1]):
    m = re.search(r'(?:D (\d+) )?(?:in\s+(-?\d) (\d) (\d) (\d)\s+(-?[\d.]+) \| )?k (\d+) pos (-?\d+) (-?\d+) v (-?[\d.]+) (-?[\d.]+) .*hook (-?\d+) .* j (\d) gr (\d) rl (\d+)', l)
    if m:
        ours[int(m.group(7))] = m
for k in range(int(sys.argv[2]), int(sys.argv[3]) + 1):
    x, y, dx, dy, ax, ay = tv.fit(k, 2)
    vx = tv.deramp(dx, dy)
    o = ours.get(k)
    s = f"k {k} T ({x:6.0f},{y:5.0f}) v({vx:5.1f},{dy:5.1f})"
    if o:
        inp = f"in {o.group(2)} {o.group(3)} {o.group(4)} {o.group(5)} {float(o.group(6)):6.1f}" if o.group(2) else ""
        s += f" | us ({o.group(8):>5},{o.group(9):>5}) v({float(o.group(10)):5.1f},{float(o.group(11)):5.1f}) h{o.group(12)} j{o.group(13)} rl{o.group(15):>2} {inp}"
    print(s)
