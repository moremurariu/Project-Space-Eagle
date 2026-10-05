# Hill-climb the post-pickup turnaround from the rt 978 state (pre978w: weapon 3 on the pickup tick) or from an earlier cut.
# usage: b1_turn.py CUTLINES YGATE N ITERS SEED [nofire]
import sys, math, time
from xl import *
from hc import *
cut = int(sys.argv[1]); ygate = float(sys.argv[2]); N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
nofire = len(sys.argv) > 6 and sys.argv[6] == 'nofire'
x = XL(); x.load('pre978w.txt', cut); x.save(0)
# initial: the rest of pf3_0 (prefix lines cut..1046 from pre978w, then pf3_0's post part)
L = [l.split() for l in open('pre978w.txt').readlines()[cut:]] + [l.split() for l in open('../runs/gren/kog1/pf3_0.txt').readlines()[1046:]]
def toaim(tx, ty):
    return math.degrees(math.atan2(ty, tx))
init = [[int(l[0]), int(l[1]), int(l[2]), 0 if nofire else int(l[3]), toaim(int(l[4]), int(l[5]))] for l in L[:N]]
sc = gate_score(ygate=ygate, xmax=5376, hold=4, tie=lambda s: s['vy'])
t = time.time()
best, seq = climb(x, 0, init, sc, iters=iters, lam=120, seed=seed, allow_fire=not nofire,
                  log=lambda it, b: print(it, b, f'{time.time()-t:.0f}s', flush=True))
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s), s['gren'])
import json
json.dump(dict(cut=cut, ygate=ygate, best=best, seq=seq), open(f'runs_b/b1_{cut}_{int(ygate)}_{seed}{"_nf" if nofire else ""}.json', 'w'))
