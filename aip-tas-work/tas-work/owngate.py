#!/usr/bin/env python3
"""owngate.py RUN K [lo] [hi]: race tick at which RUN's prefix replay first reaches reference (Teero) index K (seg's own tracker)."""
import re, subprocess, sys
run, K = sys.argv[1], int(sys.argv[2])
L = [l for l in open(run).read().split('\n') if l.strip()]
def ref(n):
    open('/tmp/owngate_p.txt', 'w').write('\n'.join(L[:n]) + '\n')
    o = subprocess.run(['../ddnet/build-sim/segf', 'AiP-Gores.map', 'prefix=/tmp/owngate_p.txt', 'gate=finish', 'maxticks=0'], capture_output=True, text=True).stdout
    return int(re.search(r'ref idx (\d+)', o).group(1))
lo, hi = 1000 + 68, len(L)
while lo < hi:
    m = (lo + hi) // 2
    if ref(m) >= K: hi = m
    else: lo = m + 1
print(lo - 68)
