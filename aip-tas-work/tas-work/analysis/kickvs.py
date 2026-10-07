#!/usr/bin/env python3
"""kickvs.py RUN: splits the speed gap to Teero into kick windows and the stretches between kicks.
For each of our explosions at rt e, the kick window is our rt e-3 .. e+4; the stretch is from there to the next kick
window. In each piece: our path-speed change and Teero's over the same part of the path (5-tick displacement / 5 at
both ends, his via the position match). Totals tell whether he gains more at the kicks (better rockets) or loses
less between them (less braking from hooks, turns, walls)."""
import math, subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
run = sys.argv[1]
T = {}
for l in open(f'{TW}/teero_track.txt'):
    a = l.split()
    if len(a) >= 3:
        T[int(a[0])] = (float(a[1]), float(a[2]))
out = subprocess.run([f'{TW}/../ddnet/build-sim/x_trace', f'{TW}/AiP-Gores.map', run, '966'], capture_output=True,
                     text=True).stdout
A, E, H = {}, [], {}
for l in out.splitlines():
    a = l.split()
    if len(a) >= 28 and a[0] == 'T':
        A[int(a[2])] = (float(a[3]), float(a[4]))
        H[int(a[2])] = int(a[9]) == 5
    elif a and a[0] == 'E':
        E.append(int(a[2]))
E = sorted(set(E))
lag, m = {}, None
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
        break
    m = best[1]
    lag[t] = m


def sp(P, k, w=5):
    k = int(round(k))
    if k - w // 2 not in P or k + w - w // 2 not in P:
        return None
    a, b = P[k - w // 2], P[k + w - w // 2]
    return math.hypot(b[0] - a[0], b[1] - a[1]) / w


cuts = []
for e in E:
    cuts += [('K', e - 3), ('S', e + 4)]
kick = [0.0, 0.0, 0]
stretch = [0.0, 0.0, 0, 0]
rows = []
for i in range(len(cuts) - 1):
    kind, t0 = cuts[i]
    t1 = cuts[i + 1][1]
    if t1 <= t0 or t0 not in lag or t1 not in lag:
        continue
    a0, a1, b0, b1 = sp(A, t0), sp(A, t1), sp(T, lag[t0]), sp(T, lag[t1])
    if None in (a0, a1, b0, b1):
        continue
    du, dh = a1 - a0, b1 - b0
    if kind == 'K':
        kick[0] += du; kick[1] += dh; kick[2] += 1
    else:
        hk = sum(1 for t in range(t0, t1) if H.get(t))
        stretch[0] += du; stretch[1] += dh; stretch[2] += 1; stretch[3] += t1 - t0
        rows.append((t0, t1, du, dh, hk))
print(f'kick windows ({kick[2]}): our speed change {kick[0]:+.1f} px/t, his {kick[1]:+.1f} (per kick {kick[0]/kick[2]:+.2f} vs {kick[1]/kick[2]:+.2f})')
print(f'between kicks ({stretch[2]}, {stretch[3]} ticks): ours {stretch[0]:+.1f}, his {stretch[1]:+.1f} (per tick {stretch[0]/stretch[3]:+.3f} vs {stretch[1]/stretch[3]:+.3f})')
print('stretches where he loses >= 3 px/t less than we do:')
for t0, t1, du, dh, hk in rows:
    if dh - du >= 3:
        print(f'  rt {t0}-{t1}: ours {du:+5.1f}, his {dh:+5.1f}  (our hook grabbed {hk}/{t1 - t0} ticks)')
