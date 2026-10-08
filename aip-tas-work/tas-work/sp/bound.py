"""Speed-profile bound on a fixed path: kicks of K px/t every P ticks (aligned), braking at B px/t per tick only where
needed to meet the speed caps at the path's sinks (local speed minima of a reference run), gravity included.
Prints the time vs the reference run's, per section."""
import sys, math
from trk import *
run = sys.argv[1]
K = float(sys.argv[2]) if len(sys.argv) > 2 else 12
P = int(sys.argv[3]) if len(sys.argv) > 3 else 25
B = float(sys.argv[4]) if len(sys.argv) > 4 else 4
capmul = float(sys.argv[5]) if len(sys.argv) > 5 else 1.0
T, E = trace(run)
ts = [t for t in sorted(T) if t >= 966]
# arc length samples
S = [0.0]; Pp = [T[ts[0]]['p']]
for a, b in zip(ts, ts[1:]):
    S.append(S[-1] + math.dist(T[a]['p'], T[b]['p'])); Pp.append(T[b]['p'])
V = [T[t]['s'] for t in ts]
L = S[-1]
# sinks: local minima of speed over +-15 ticks with a drop
caps = []
for i in range(len(ts)):
    lo, hi = max(0, i - 15), min(len(ts), i + 16)
    if V[i] == min(V[lo:hi]) and max(V[max(0, i - 60):i + 1]) - V[i] > 8:
        caps.append((S[i], V[i] * capmul, ts[i]))
print('sinks:', ' '.join('%d:%.0f' % (c[2], c[1]) for c in caps))
def ramp(v): return 1.0 if v * 50 < 550 else 1.4 ** (-(v * 50 - 550) / 2000)
def at(s):  # tangent angle and y at arc length s
    j = max(0, min(len(S) - 2, next((k for k in range(len(S) - 1) if S[k + 1] >= s), len(S) - 2)))
    a, b = Pp[j], Pp[j + 1]
    d = math.dist(a, b) or 1e-9
    u = (s - S[j]) / (S[j + 1] - S[j] or 1e-9)
    return ((b[0] - a[0]) / d, (b[1] - a[1]) / d), a[1] + (b[1] - a[1]) * u
# cap profile by distance: backward pass on a fine grid
N = int(L) + 1
cap = [1e9] * (N + 1)
for s, v, _ in caps:
    cap[int(s)] = min(cap[int(s)], v)
# braking: dv/ds = -B / disp(v) ~ -B / (v*0.8); do backward in s
for i in range(N - 1, -1, -1):
    v = cap[i + 1]
    if v < 1e8:
        dvds = B / max(5.0, v * 0.8)
        cap[i] = min(cap[i], v + dvds)
# forward sim
s, v, t = 0.0, V[0], 0
y0 = at(0)[1]
last = -P
out = []
ycur = y0
while s < L and t < 5000:
    (tx, ty), y = at(s)
    # gravity: v^2 += dy (descending)
    v2 = v * v + (y - ycur); ycur = y
    v = math.sqrt(max(1.0, v2))
    if t - last >= P and v + K <= cap[min(N, int(s) + 1)] + 0.01 + 1e9:
        # kick only if not immediately braked away: allow if cap ahead (within 15 ticks) permits most of it
        ahead = min(cap[min(N, int(s + k))] for k in range(0, int(15 * v * 0.8) + 1, 8))
        if ahead > v + K * 0.5:
            v += K; last = t
    v = min(v, cap[min(N, int(s))])
    d = v * math.hypot(tx * ramp(v), ty)
    s += d; t += 1
    out.append((t, s))
print('path %.0f px, model %d ticks vs run %d (K %.0f P %d B %.1f capmul %.2f)' % (L, t, len(ts) - 1, K, P, B, capmul))
# per section comparison every 200 run ticks
j = 0
for i in range(0, len(ts), 100):
    s_i = S[i]
    while j < len(out) - 1 and out[j][1] < s_i: j += 1
    print('rt %d s %6.0f run %4d model %4d  v %.0f' % (ts[i], s_i, i, out[j][0], V[i]))
# curvature caps: radius from velocity direction change of the run per tick (heading change / displacement)
alat = float(sys.argv[6]) if len(sys.argv) > 6 else 0
if alat > 0:
    cap2 = cap[:]
    for i in range(1, len(ts) - 1):
        va, vb = T[ts[i - 1]]['v'], T[ts[i + 1]]['v']
        a1, a2 = math.atan2(va[1], va[0]), math.atan2(vb[1], vb[0])
        dth = abs((a2 - a1 + math.pi) % (2 * math.pi) - math.pi) / 2
        ds = max(1.0, (S[i + 1] - S[i - 1]) / 2)
        if dth > 1e-4:
            R = ds / dth
            vmax = math.sqrt(alat * R)
            for k in range(int(S[i]), min(N, int(S[i + 1]) + 1)):
                cap2[k] = min(cap2[k], vmax)
    for i in range(N - 1, -1, -1):
        v = cap2[i + 1]
        cap2[i] = min(cap2[i], v + B / max(5.0, v * 0.8))
    s, v, t, last, ycur = 0.0, V[0], 0, -P, y0
    vs = []
    while s < L and t < 5000:
        (tx, ty), y = at(s)
        v = math.sqrt(max(1.0, v * v + (y - ycur))); ycur = y
        if t - last >= P:
            ahead = min(cap2[min(N, int(s + k))] for k in range(0, int(15 * v * 0.8) + 1, 8))
            if ahead > v + K * 0.5:
                v += K; last = t
        v = min(v, cap2[min(N, int(s))])
        s += v * math.hypot(tx * ramp(v), ty); t += 1; vs.append(v)
    print('with curvature caps (alat %.1f): %d ticks, mean v %.1f max %.1f' % (alat, t, sum(vs) / len(vs), max(vs)))
