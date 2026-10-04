#!/usr/bin/env python3
"""Imitation window: hill-climb inputs from RT0 (hook idle) to follow Teero's track shifted by LAG ticks
(target position at our race tick r = Teero label r + 3 - LAG ... i.e. true tick r - LAG), smoothed track.
usage: track_win.py RUN RT0 NT LAG ITERS SEED OUT"""
import sys, random, math
from labx import *
from seqopt2 import *
run_f, RT0, NT, LAG, iters, seed, out_f = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), float(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), sys.argv[7]
T = {}
for l in open(os.path.join(WORK, 'teero_track.txt')):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
def tpos(true_t):  # Teero position at a (fractional) true race tick, 5-tick smoothing
    k = true_t + 3
    k0 = math.floor(k); f = k - k0
    def sm(kk):
        ks = [kk + d for d in range(-2, 3)]
        return (sum(T[i][0] for i in ks) / 5, sum(T[i][1] for i in ks) / 5)
    a, b = sm(k0), sm(k0 + 1)
    return (a[0] + f * (b[0] - a[0]), a[1] + f * (b[1] - a[1]))
L = read_inputs(run_f)
n0 = RT0 + 68
st = state_after(run_f, n0)
N = NT
def obj(S):
    c = 0
    for i, s in enumerate(S):
        if s['frz']:
            return 1e7 + (N - i) * 1e5
        tx, ty = tpos(RT0 + 1 + i - LAG)
        c += (s['x'] - tx) ** 2 + (s['y'] - ty) ** 2
    return c / N
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
base = L[n0:n0 + N]
cand = from_inputs3(base)
(J0,), _ = evalf([cand])
print('base track cost', J0, flush=True)
J, cur, S = hill3(evalf, cand, N, iters=iters, pop=300, rng=random.Random(seed), first_free=0, log=False, patience=40)
print('best track cost', J, '(rms px %.1f)' % math.sqrt(J))
with open(out_f, 'w') as f:
    for c in L[:n0] + to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
for i in range(0, N, 4):
    s = S[i]; tx, ty = tpos(RT0 + 1 + i - LAG)
    print(f"rt {RT0+1+i} ours ({s['x']:.0f},{s['y']:.0f}) v({s['vx']:.1f},{s['vy']:.1f}) E {E(s):.0f} j{s['jumped']} g{s['gr']} | Teero@true {RT0+1+i-LAG:.0f} ({tx:.0f},{ty:.0f})")
