#!/usr/bin/env python3
"""lagvs.py A B [from_rt] [to_rt] [step]: lag of run A behind run B at the same place (A's race tick - B's matched
tick, by projecting A's positions onto B's path) every `step` ticks, with both speeds. Negative = A ahead."""
import math, subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
r0 = int(sys.argv[3]) if len(sys.argv) > 3 else 980
r1 = int(sys.argv[4]) if len(sys.argv) > 4 else 9999
st = int(sys.argv[5]) if len(sys.argv) > 5 else 5


def tr(path):
    out = subprocess.run([f'{TW}/../ddnet/build-sim/x_trace', f'{TW}/AiP-Gores.map', path, str(r0 - 60)], capture_output=True, text=True).stdout
    A = {}
    for l in out.splitlines():
        a = l.split()
        if len(a) >= 28 and a[0] == 'T':
            A[int(a[2])] = (float(a[3]), float(a[4]), float(a[7]), 'DEAD' in a)
    return A


A, B = tr(sys.argv[1]), tr(sys.argv[2])
m = None
for t in sorted(A):
    if t < r0 - 5:
        continue
    p, best = A[t], None
    for r in (range(t - 30, t + 30) if m is None else range(int(m) - 6, int(m) + 12)):
        if r in B and r + 1 in B:
            a, b = B[r], B[r + 1]
            dx, dy = b[0] - a[0], b[1] - a[1]
            u = max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
            d = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
            if best is None or d < best[0]:
                best = (d, r + u)
    if best is None:
        break
    m = best[1]
    if r0 <= t <= r1 and t % st == 0:
        print('%d lag %+6.2f |v| %5.1f vs %5.1f off %4.0f%s' % (t, t - m, p[2], B[int(m)][2], best[0], ' DEAD' if p[3] else ''))
