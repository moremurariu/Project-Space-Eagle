# Downstream hill climb with Teero-track gates: gate k (labels K0..K1 step 2) = x >= teero_x(k) with |y - teero_y(k)| < YW.
# Score = (-gates passed, fractional rt of the last gate passed). Alive through the whole sequence required.
# usage: d2_down.py PREFIX PRELINES SEEDFILE SEEDFROM N ITERS SEED K1 OUT [K0]
import sys, math, time, json, random
from xl import *
from hc import *
pre, prel, seedf, seedfrom, N, iters, seed, K1, out = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7]), int(sys.argv[8]), sys.argv[9]
K0 = int(sys.argv[10]) if len(sys.argv) > 10 else None
YW = 140
T = {}
for l in open('../teero_track.txt'):
    p = l.split()
    T[int(p[0])] = (float(p[1]), float(p[2]))
x = XL(); x.load(pre, prel); x.save(0)
s0 = x.state()
if K0 is None:
    K0 = min(k for k in T if k > 1000 and T[k][0] > s0['x'] + 20)
GATES = [(k, T[k][0], T[k][1]) for k in range(K0, K1 + 1, 2)]
if GATES[-1][0] != K1:
    GATES.append((K1, T[K1][0], T[K1][1]))


def toaim(tx, ty):
    return math.degrees(math.atan2(ty, tx))


if seedf.endswith('.json'):
    init = json.load(open(seedf))['seq']
else:
    L = [l.split() for l in open(seedf).readlines()[seedfrom:]]
    init = [[int(l[0]), int(l[1]), int(l[2]), int(l[3]), toaim(int(l[4]), int(l[5]))] for l in L]
init = (init + [[1, 0, 0, 0, 0.0]] * N)[:N]


def score(S):
    g = 0
    tl = 0.0
    prev = s0
    for s in S:
        if s['frz']:
            return None
        while g < len(GATES) and s['x'] >= GATES[g][1] and abs(s['y'] - GATES[g][2]) < YW:
            frac = (s['x'] - GATES[g][1]) / max(1.0, s['x'] - prev['x'])
            tl = s['rt'] - frac
            g += 1
        prev = s
    return (-g, tl)


t0 = time.time()
best, seq = climb(x, 0, init, score, iters=iters, lam=100, seed=seed, nfire_max=5,
                  log=lambda it, b: print(it, b, f'{time.time()-t0:.0f}s', flush=True) if it % 25 == 0 else None)
print('BEST', best, 'gates', len(GATES), 'last gate label', GATES[-1][0])
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s))
json.dump(dict(pre=pre, prel=prel, best=best, seq=seq), open(out, 'w'))
