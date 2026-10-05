# Downstream hill climb: from a full prefix file (cut after PRELINES lines), optimise N ticks of inputs (shots allowed)
# for the earliest (fractional) race tick with x >= XG (alive through the sequence end).
# usage: d1_down.py PREFIX PRELINES SEEDFILE SEEDFROM N ITERS SEED XG OUT
#   SEEDFILE/SEEDFROM: initial inputs = lines of SEEDFILE starting at line index SEEDFROM
import sys, math, time, json, random
from xl import *
from hc import *
pre, prel, seedf, seedfrom, N, iters, seed, XG, out = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7]), float(sys.argv[8]), sys.argv[9]
x = XL(); x.load(pre, prel); x.save(0)
def toaim(tx, ty):
    return math.degrees(math.atan2(ty, tx))
if seedf.endswith('.json'):
    init = json.load(open(seedf))['seq']
else:
    L = [l.split() for l in open(seedf).readlines()[seedfrom:]]
    init = [[int(l[0]), int(l[1]), int(l[2]), int(l[3]), toaim(int(l[4]), int(l[5]))] for l in L]
init = (init + [[1, 0, 0, 0, 0.0]] * N)[:N]
def score(S):
    hit = None
    for i, s in enumerate(S):
        if s['frz']:
            return None
        if hit is None and s['x'] >= XG:
            p = S[i - 1] if i > 0 else s
            frac = (s['x'] - XG) / max(1.0, s['x'] - p['x'])
            hit = s['rt'] - frac
    if hit is None:
        e = S[-1]
        return (5000 - e['x'] / 30.0, 0)
    e = S[-1]
    return (hit, -math.hypot(e['vx'], e['vy']) * 0.001)
t0 = time.time()
best, seq = climb(x, 0, init, score, iters=iters, lam=100, seed=seed, nfire_max=4,
                  log=lambda it, b: print(it, b, f'{time.time()-t0:.0f}s', flush=True) if it % 25 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s))
json.dump(dict(pre=pre, prel=prel, best=best, seq=seq), open(out, 'w'))
