#!/usr/bin/env python3
"""stretchvs.py RUN OTHER [from] [to] [min=0.8]: stretches where OTHER gains on RUN at the same place (analysis/lagvs.py,
points within 10 px of OTHER's path, lag jumps > 2 between 5-tick samples dropped), with RUN's lag at the start."""
import subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
run, oth = sys.argv[1], sys.argv[2]
lo = sys.argv[3] if len(sys.argv) > 3 else '1000'
hi = sys.argv[4] if len(sys.argv) > 4 else '2540'
mn = float(sys.argv[5]) if len(sys.argv) > 5 else 0.8
out = subprocess.run(['python3', f'{TW}/analysis/lagvs.py', run, oth, lo, hi, '5'], capture_output=True, text=True).stdout
G = []
for l in out.splitlines():
    a = l.split()
    if 'off' not in a:
        continue
    off, lag = float(a[a.index('off') + 1]), float(a[2])
    if off < 10 and (not G or abs(lag - G[-1][1]) < 2):
        G.append((int(a[0]), lag))
segs, i = [], 0
while i < len(G) - 1:
    j = i
    while j + 1 < len(G) and G[j + 1][1] >= G[j][1] - 0.15:
        j += 1
    if G[j][1] - G[i][1] >= mn:
        segs.append((G[i][0], G[j][0], G[j][1] - G[i][1], G[i][1]))
    i = j + 1
print(oth, ' '.join('%d-%d(+%.1f,lag%+.1f)' % s for s in segs), '| total %.1f' % sum(s[2] for s in segs))
