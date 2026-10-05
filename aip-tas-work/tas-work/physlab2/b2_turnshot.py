# Hill-climb the turnaround from rt 978 with an early brake shot seeded (fire at 979 aim A), several seeds.
import sys, math, time, json
from xl import *
from hc import *
ygate = float(sys.argv[1]); N = int(sys.argv[2]); iters = int(sys.argv[3]); seed = int(sys.argv[4]); A = float(sys.argv[5])
x = XL(); x.load('pre978w.txt'); x.save(0)
init = []
for k in range(N):
    init.append([-1, 1 if k == 8 else 0, 0 if k == 7 else 1, 1 if k == 0 else 0, A if k == 0 else (265.0 if k >= 7 else -95.6)])
sc = gate_score(ygate=ygate, xmax=5376, hold=4, tie=lambda s: s['vy'])
t = time.time()
best, seq = climb(x, 0, init, sc, iters=iters, lam=120, seed=seed, nfire_max=1,
                  log=lambda it, b: print(it, b, f'{time.time()-t:.0f}s', flush=True) if it % 60 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s))
json.dump(dict(ygate=ygate, best=best, seq=seq), open(f'runs_b/b2_{int(ygate)}_{seed}_{int(A)}.json', 'w'))
