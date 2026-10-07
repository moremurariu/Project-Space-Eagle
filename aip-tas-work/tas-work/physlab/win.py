#!/usr/bin/env python3
"""Window optimizer: hill-climb the inputs of RUN from race tick RT0 (state after input RT0, hook must be idle)
to a gate, exact lab evaluation.  usage: win.py RUN RT0 NT GATE LAM ITERS SEED [OUT]
GATE: x>=X[,ylo,yhi] | y>=Y[,xlo,xhi] | x<=X[,ylo,yhi]   objective: t_gate - LAM*E_gate  (needs 6 more alive ticks)"""
import sys, os, random, math
from labx import *
from seqopt2 import *
run_f, RT0, NT, gate, lam, iters, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], float(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7])
out_f = sys.argv[8] if len(sys.argv) > 8 else None
L = read_inputs(run_f)
n0 = RT0 + 68
st = state_after(run_f, n0)
assert st['hook'] in (0, -1), 'hook not idle at window start: %s' % fmt(st)
g = gate.replace('>=', ' ge ').replace('<=', ' le ').split()
axis, op, rest = g[0], g[1], [float(v) for v in g[2].split(',')]
G = rest[0]; lo, hi = (rest[1], rest[2]) if len(rest) > 2 else (-1e9, 1e9)
oth = 'y' if axis == 'x' else 'x'
def cross(S):
    for i, s in enumerate(S):
        if s['frz']: return None, i
        v = s[axis]
        if (v >= G if op == 'ge' else v <= G) and lo <= s[oth] <= hi:
            p = S[i - 1][axis] if i else v - s['v' + axis]
            f = (G - p) / (v - p) if v != p else 1.0
            return RT0 + i + f, i
    return None, len(S)
def obj(S):
    t, i = cross(S)
    if t is None:
        alive = next((k for k, s in enumerate(S) if s['frz']), len(S))
        return 10000 - alive
    for s in S[i:i + 6]:
        if s['frz']: return 9000
    return t - lam * E(S[i])
N = NT
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
base = L[n0:n0 + N]
while len(base) < N: base.append(base[-1])
cand = from_inputs3(base)
(J0,), (S0,) = evalf([cand])
t0, i0 = cross(S0)
print(f'base: J {J0:.3f} t {t0} {fmt(S0[i0]) if t0 else ""}', flush=True)
J, cur, S = hill3(evalf, cand, N, iters=iters, pop=300, rng=random.Random(seed), first_free=0, log=False, patience=30)
t, i = cross(S)
print(f'best: J {J:.3f} t {t} {fmt(S[i]) if t else ""}')
if out_f:
    full = L[:n0] + to_inputs3(*cur, N)[:i + 8]
    with open(out_f, 'w') as f:
        for c in full: f.write(' '.join(map(str, c)) + '\n')
    # verify by full replay
    S2 = run(os.path.abspath(os.path.join(TMP, 'empty.txt')), full)
    s2 = [s for s in S2 if s['rt'] == S[i]['rt'] + 0] if False else None
    print('wrote', out_f)
