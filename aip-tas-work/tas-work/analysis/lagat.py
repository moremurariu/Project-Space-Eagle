#!/usr/bin/env python3
"""lagat.py RUN [from_rt] [to_rt] [step]: our lag behind Teero (our race tick - his matched label, by projecting
our position onto teero_track.txt) every `step` ticks, with our |v| and the matched distance. A rising lag = he gains."""
import math, subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
run = sys.argv[1]
r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 980
r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 9999
st = int(sys.argv[4]) if len(sys.argv) > 4 else 5
T = {}
for l in open(f'{TW}/teero_track.txt'):
    a = l.split()
    if len(a) >= 3:
        T[int(a[0])] = (float(a[1]), float(a[2]))
out = subprocess.run([f'{TW}/../ddnet/build-sim/x_trace', f'{TW}/AiP-Gores.map', run, str(min(r0, 966))],
                     capture_output=True, text=True).stdout
A = {}
for l in out.splitlines():
    a = l.split()
    if len(a) >= 28 and a[0] == 'T':
        A[int(a[2])] = (float(a[3]), float(a[4]), float(a[7]))
m = None
for t in sorted(A):
    p, best = A[t], None
    for r in (range(t - 40, t + 10) if m is None else range(int(m) - 8, int(m) + 12)):
        if r in T and r + 1 in T:
            a, b = T[r], T[r + 1]
            dx, dy = b[0] - a[0], b[1] - a[1]
            u = max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
            d = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
            if best is None or d < best[0]:
                best = (d, r + u)
    if best is None:
        if m is None:
            continue
        break
    m = best[1]
    if r0 <= t <= r1 and t % st == 0:
        print('%d lag %+6.2f |v| %5.1f off %4.0f' % (t, t - m, p[2], best[0]))
