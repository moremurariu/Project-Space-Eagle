# Stage 2 with the analytic double-kick model: hill-climb the climb (no shots) to minimise the double-kick tick.
# usage: c2_climb.py CUTLINES INITJSON N ITERS SEED
import sys, math, time, json
from xl import *
from hc import *
from gmodel import double_kick_T
cut = int(sys.argv[1]); initf = sys.argv[2]; N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
x = XL(); x.load('pre978w.txt', cut); x.save(0)
d = json.load(open(initf))
init = (d['seq'] + [[-1, 0, 1, 0, 250.0]] * N)[:N]
for c in init:
    c[3] = 0
def score(S):
    for s in S:
        if s['frz']:
            return None
    r = double_kick_T(S)
    if r is None:
        return (9999, 0)
    e, m, n, a, yc = r
    t = S[e - 1]
    # survival margin: alive 3 more ticks in the base; tie: upward speed near -10, low x (closer to face)
    return (S[e]['rt'], abs(t['vy'] + 10) * 0.05 + (t['x'] - 5281) * 0.01)
t0 = time.time()
best, seq = climb(x, 0, init, score, iters=iters, lam=100, seed=seed, allow_fire=False,
                  log=lambda it, b: print(it, b, f'{time.time()-t0:.0f}s', flush=True) if it % 25 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
print('model', double_kick_T(S))
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s), 'g', s['gren'], 'hk', s['hx'], s['hy'])
json.dump(dict(cut=cut, best=best, seq=seq), open(f'runs_b/c2_{cut}_{seed}.json', 'w'))
