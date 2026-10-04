#!/usr/bin/env python3
"""Random multi-parameter perturbations of a run window (all hook segments / jumps / dir blocks jittered at once),
exact evaluation with lab batch, then hill climbing of the best few.
usage: perturb.py RUN RT0 NT GATE LAM NS SEED [P]   (GATE as in win.py; P = per-element perturb probability)"""
import sys, os, random, math
from labx import *
from seqopt2 import *
run_f, RT0, NT, gate, lam, NS, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], float(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7])
P = float(sys.argv[8]) if len(sys.argv) > 8 else 0.3
rng = random.Random(seed)
L = read_inputs(run_f); n0 = RT0 + 68
st = state_after(run_f, n0)
assert st['hook'] in (0, -1)
g = gate.replace('>=', ' ge ').replace('<=', ' le ').split()
axis, op, rest = g[0], g[1], [float(v) for v in g[2].split(',')]
G = rest[0]; lo, hi = (rest[1], rest[2]) if len(rest) > 2 else (-1e9, 1e9)
oth = 'y' if axis == 'x' else 'x'
N = NT
def cross(S):
    for i, s in enumerate(S):
        if s['frz']: return None, i
        v = s[axis]
        if (v >= G if op == 'ge' else v <= G) and lo <= s[oth] <= hi:
            p = S[i - 1][axis] if i else v - s['v' + axis]
            return RT0 + i + ((G - p) / (v - p) if v != p else 1.0), i
    return None, len(S)
def obj(S):
    t, i = cross(S)
    if t is None:
        return 10000 - next((k for k, s in enumerate(S) if s['frz']), len(S))
    for s in S[i:i + 6]:
        if s['frz']: return 9000
    return t - lam * E(S[i])
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
base = from_inputs3(L[n0:n0 + N])
(J0,), (S0,) = evalf([base]); t0, i0 = cross(S0)
print(f'base J {J0:.3f} t {t0}', flush=True)
def perturb(c):
    d, s, j = list(c[0]), [list(x) for x in c[1]], list(c[2])
    for x in s:
        if rng.random() < P: x[0] += rng.randint(-2, 2)
        if rng.random() < P: x[1] += rng.randint(-3, 3)
        if rng.random() < P: x[2] += rng.uniform(-6, 6)
    if rng.random() < P * 0.5 and s: s.pop(rng.randrange(len(s)))
    for k in range(len(j)):
        if rng.random() < P: j[k] += rng.randint(-4, 4)
    if rng.random() < P * 0.3 and j: j.pop(rng.randrange(len(j)))
    if rng.random() < P * 0.3: j.append(rng.randrange(N))
    for _ in range(rng.randint(0, 2)):
        a = rng.randrange(N); v = rng.choice([-1, 0, 1])
        for q in range(a, min(N, a + rng.randint(1, 3))): d[q] = v
    s = sorted((max(0, x[0]), x[1], x[2]) for x in s if x[1] > x[0])
    # drop overlaps
    out = []
    for x in s:
        if out and x[0] <= out[-1][1]: continue
        out.append(x)
    j = sorted(set(min(N - 1, max(0, q)) for q in j))
    return d, out, j
cands = [perturb(base) for _ in range(NS)]
J, B = evalf(cands)
order = sorted(range(len(J)), key=lambda i: J[i])
print('alive', sum(1 for x in J if x < 9000), 'of', len(J), 'better than base', sum(1 for x in J if x < J0), flush=True)
for k in order[:5]:
    t, i = cross(B[k]); print(f'  J {J[k]:.3f} t {t} {fmt(B[k][i]) if t else ""}', flush=True)
best = (J0, base, S0)
for r, k in enumerate(order[:3]):
    Jh, cur, S = hill3(evalf, cands[k], N, iters=60, pop=300, rng=random.Random(r), first_free=0, log=False, patience=15)
    t, i = cross(S)
    print(f'HILL {r}: J {Jh:.3f} t {t} {fmt(S[i]) if t else ""}', flush=True)
    if Jh < best[0]: best = (Jh, cur, S)
out = f'runs_q2/perturb_{RT0}_{gate.replace(">=", "ge").replace(",", "_")}_{seed}.txt'
J, cur, S = best; t, i = cross(S)
with open(out, 'w') as f:
    for c in L[:n0] + to_inputs3(*cur, N)[:i + 8]: f.write(' '.join(map(str, c)) + '\n')
print('best', J, t, 'wrote', out)
