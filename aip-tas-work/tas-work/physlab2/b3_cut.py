# Hill-climb from an earlier cut (prefix lines) through the pickup to y<=YGATE (with grenade, x<=5376).
# usage: b3_cut.py CUTLINES YGATE N ITERS SEED [initjson]
import sys, math, time, json
from xl import *
from hc import *
cut = int(sys.argv[1]); ygate = float(sys.argv[2]); N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
x = XL(); x.load('pre978w.txt', cut); x.save(0)
def toaim(tx, ty):
    return math.degrees(math.atan2(ty, tx))
L = [l.split() for l in open('pre978w.txt').readlines()[cut:]]
init = [[int(l[0]), int(l[1]), int(l[2]), int(l[3]), toaim(int(l[4]), int(l[5]))] for l in L]
if len(sys.argv) > 6:
    init += json.load(open(sys.argv[6]))['seq']
init = (init + [[-1, 0, 1, 0, 265.0]] * N)[:N]
sc = gate_score(ygate=ygate, xmax=5376, hold=4, tie=lambda s: s['vy'])
t = time.time()
best, seq = climb(x, 0, init, sc, iters=iters, lam=120, seed=seed, nfire_max=2,
                  log=lambda it, b: print(it, b, f'{time.time()-t:.0f}s', flush=True) if it % 50 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s), 'g', s['gren'])
json.dump(dict(cut=cut, ygate=ygate, best=best, seq=seq), open(f'runs_b/b3_{cut}_{int(ygate)}_{seed}.json', 'w'))
