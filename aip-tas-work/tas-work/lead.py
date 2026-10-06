#!/usr/bin/env python3
"""lead of run B over run A along A's path: for B's ticks every STEP, A's tick at the nearest point (monotone).
usage: lead.py A B RT0 RT1 [STEP=25]"""
import math, subprocess, sys
def tr(f):
    d = {}
    for l in subprocess.run(['../ddnet/build-sim/x_trace', 'AiP-Gores.map', f], capture_output=True, text=True, cwd=sys.path[0]).stdout.splitlines():
        if l.startswith('T '):
            a = l.split(); d[int(a[2])] = (float(a[3]), float(a[4]))
    return d
A, Bf, r0, r1 = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4])
st = int(sys.argv[5]) if len(sys.argv) > 5 else 25
I, R = tr(A), tr(Bf)
ik = sorted(I)
j = max(0, ik.index(r0) - 20)
for rt in range(r0, r1 + 1):
    if rt not in R: break
    q = R[rt]; best = 1e18; bj = j; ff = 0
    for k in range(max(0, j - 5), min(len(ik) - 1, j + 80)):
        P, Q = I[ik[k]], I[ik[k + 1]]
        abx, aby = Q[0] - P[0], Q[1] - P[1]; l2 = abx * abx + aby * aby
        f = max(0, min(1, ((q[0] - P[0]) * abx + (q[1] - P[1]) * aby) / l2)) if l2 else 0
        dd = math.hypot(q[0] - P[0] - abx * f, q[1] - P[1] - aby * f)
        if dd < best: best, bj, ff = dd, k, f
    j = bj
    if (rt - r0) % st == 0:
        print(f'rt {rt}: A rt {ik[bj] + ff:7.1f} lead {ik[bj] + ff - rt:+5.1f} (d {best:3.0f})')
