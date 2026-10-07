#!/usr/bin/env python3
"""speedgap.py RUN [step]: speed at the same place on the path, ours vs Teero's (10-tick displacement / 10, which
averages out the video track's jitter), every `step` of our race ticks, with what our run does there (kicks |f|/cos,
hook grabbed ticks, jumps). The gap's changes show the events where he gains speed on us."""
import math, subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
run = sys.argv[1]
step = int(sys.argv[2]) if len(sys.argv) > 2 else 10
T = {}
for l in open(f'{TW}/teero_track.txt'):
    a = l.split()
    if len(a) >= 3:
        T[int(a[0])] = (float(a[1]), float(a[2]))
out = subprocess.run([f'{TW}/../ddnet/build-sim/x_trace', f'{TW}/AiP-Gores.map', run, '966'], capture_output=True,
                     text=True).stdout
A, E = {}, {}
for l in out.splitlines():
    a = l.split()
    if len(a) >= 28 and a[0] == 'T':
        A[int(a[2])] = dict(p=(float(a[3]), float(a[4])), v=(float(a[5]), float(a[6])), hs=int(a[9]), jump=int(a[15]),
                            dir=int(a[14]))
    elif a and a[0] == 'E':
        E.setdefault(int(a[2]), []).append((float(a[a.index('|f|') + 1]), float(a[a.index('cos') + 1])))
lag, m = {}, None
for t in sorted(A):
    p, best = A[t]['p'], None
    for r in (range(t - 40, t + 10) if m is None else range(int(m) - 8, int(m) + 12)):
        if r in T and r + 1 in T:
            a, b = T[r], T[r + 1]
            dx, dy = b[0] - a[0], b[1] - a[1]
            u = max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
            d = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
            if best is None or d < best[0]:
                best = (d, r + u)
    if best is None:
        break
    m = best[1]
    lag[t] = m


def sp(P, k, w=10):
    k = int(round(k))
    if k - w // 2 not in P or k + w // 2 not in P:
        return None
    a, b = P[k - w // 2], P[k + w // 2]
    return math.hypot(b[0] - a[0], b[1] - a[1]) / w


Ap = {t: A[t]['p'] for t in A}
ts = sorted(lag)
print(' rt   lag  | us   him  gap | |v| | our events in the next step ticks')
for t in range(ts[0] + 5, ts[-1] - 5, step):
    su, sh = sp(Ap, t), sp(T, lag[t])
    if su is None or sh is None:
        continue
    ev = []
    for u in range(t, t + step):
        for f, c in E.get(u, []):
            ev.append(f'K{u}:{f:.0f}/{c:.2f}')
        if u in A and A[u]['jump'] and (u - 1 not in A or not A[u - 1]['jump']):
            ev.append(f'J{u}')
    hk = sum(1 for u in range(t, t + step) if u in A and A[u]['hs'] == 5)
    if hk:
        ev.append(f'hook{hk}')
    v = math.hypot(*A[t]['v'])
    print(f'{t} {t - lag[t]:+5.1f} | {su:4.1f} {sh:4.1f} {sh - su:+5.1f} | {v:4.1f} | {" ".join(ev)}')
