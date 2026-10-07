#!/usr/bin/env python3
"""lag.py TRACE: for Teero's track ticks (every 5), Teero's geodesic distance D_k to the finish and the first tick
our trace reaches D <= D_k; lag = ours - k (negative = ahead)."""
import sys, re
TD = []
for l in open('teero_D.txt'):
    m = re.match(r'k (\d+) pos \S+ \S+ D (\S+)', l)
    if m: TD.append((int(m.group(1)), float(m.group(2))))
ours = []
for l in open(sys.argv[1]):
    m = re.search(r'D (\d+) .*k (\d+) pos', l)
    if m: ours.append((int(m.group(2)), float(m.group(1))))
step = int(sys.argv[2]) if len(sys.argv) > 2 else 5
out = []
for k, d in TD:
    if k % step: continue
    t = next((kk for kk, dd in ours if dd <= d), None)
    out.append(f"k{k}:{'-' if t is None else t - k:+}" if t is not None else f"k{k}:--")
print(' '.join(out))
