# Turnaround from an earlier cut: earliest rt with y <= YG, x <= 5376, grenade active, reload 0 (no shots), climbing (vy <= -12).
# usage: b4_turn.py CUT INITJSON|none N ITERS SEED YG
import sys, math, time, json
from xl import *
from hc import *
cut = int(sys.argv[1]); initf = sys.argv[2]; N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5]); YG = float(sys.argv[6])
x = XL(); x.load('pre978w.txt', cut); x.save(0)
def toaim(tx, ty):
    return math.degrees(math.atan2(ty, tx))
L = [l.split() for l in open('pre978w.txt').readlines()[cut:1038]]
init = [[int(l[0]), int(l[1]), int(l[2]), 0, toaim(int(l[4]), int(l[5]))] for l in L]
if initf != 'none':
    d = json.load(open(initf))
    if d['cut'] == cut:
        init = [list(c) for c in d['seq']]
    else:
        assert d['cut'] == 1038
        init += d['seq']
init = (init + [[-1, 0, 1, 0, 265.0]] * N)[:N]
import os
KEEP = int(os.environ.get('KEEPFIRE', '0'))
BRAKE = os.environ.get('BRAKE')   # "aim": force a brake shot on the first tick
for i, c in enumerate(init):
    if i >= KEEP:
        c[3] = 0
if BRAKE is not None:
    init[0][3] = 1; init[0][4] = float(BRAKE); init[0][2] = 1
sc = gate_score(ygate=YG, xmax=5376, hold=6, cond=lambda s: s['gren'] == 2 and s['reload'] == 0 and s['vy'] <= -12, tie=lambda s: s['y'])
t0 = time.time()
import hc as _hc
_orig = _hc.mutate
def _mut(seq, rng, aims=None, nfire_max=3, allow_fire=True):
    m = _orig(seq, rng, aims, nfire_max, allow_fire)
    for i in range(KEEP):
        m[i][3] = init[i][3]
        if init[i][3]:
            m[i][4] = init[i][4] + (rng.uniform(-3, 3) if rng.random() < 0.2 else 0) if False else m[i][4]
    return m
_hc.mutate = _mut
best, seq = climb(x, 0, init, sc, iters=iters, lam=120, seed=seed, allow_fire=False,
                  log=lambda it, b: print(it, b, f'{time.time()-t0:.0f}s', flush=True) if it % 50 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s), 'g', s['gren'])
json.dump(dict(cut=cut, best=best, seq=seq), open(f'runs_b/b4_{cut}_{int(YG)}_{seed}.json', 'w'))
