# Turnaround from a cut of the new prefix: earliest rt with y <= YG, x <= 5376, grenade active, reload 0,
# climbing (vy <= -12), alive 6 more ticks; tie: lower y.
# init = landing seq (s1 json entry K, or a b5 json) + OLDFILE lines from old rt OLDRT+1 on (shifted).
# usage: b5_turn.py PREFIX CUT INIT(json[:K]) OLDFILE OLDRT N ITERS SEED YG [TAG]
import sys, math, time, json, random
from xl import *
from hc5 import *
from hc import gate_score
pre, cut, initf, oldf, oldrt, N, iters, seed, YG = sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4], int(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7]), int(sys.argv[8]), float(sys.argv[9])
tag = sys.argv[10] if len(sys.argv) > 10 else ''
x = XL(); x.load(pre, cut + 68); x.save(0)
if ':' in initf:
    f, k = initf.split(':')
    init = [norm(c) for c in json.load(open(f))[int(k)]['seq']]
else:
    d = json.load(open(initf))
    assert d['cut'] == cut
    init = [norm(c) for c in d['seq']]
if oldf != 'none':
    init += from_lines(open(oldf).readlines()[oldrt + 68:])
init = (init + [[-1, 0, 1, 0, 265.0, None]] * N)[:N]
for c in init:
    c[3] = 0
sc = gate_score(ygate=YG, xmax=5376, hold=6, cond=lambda s: s['gren'] == 2 and s['reload'] == 0 and s['vy'] <= -12, tie=lambda s: s['y'])
import os
if os.environ.get('YFRAC'):
    _g = sc
    def sc(S):
        r = _g(S)
        if r is None:
            return None
        i = next(k for k, s in enumerate(S) if s['rt'] == r[0])
        p = S[i - 1]
        return (p['rt'] + (p['y'] - YG) / (p['y'] - S[i]['y']),)

rng = random.Random(seed)
R = x.runmany([tuples(init)], restore=0)
best = sc(R[0]); bestseq = init
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(120):
        m = bestseq
        for _ in range(rng.choice((1, 1, 1, 2, 3))):
            m = mutate(m, rng, None, 0, False)
        cands.append(m)
    R = x.runmany([tuples(c) for c in cands], restore=0)
    for c, S in zip(cands, R):
        v = sc(S)
        if v is not None and (best is None or v <= best):
            best, bestseq = v, c
    if it % 50 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
S = x.runmany([tuples(bestseq)], restore=0)[0]
for c, s in zip(bestseq, S):
    print(line(c).ljust(30), '|', fmt(s), 'g', s['gren'])
json.dump(dict(cut=cut, best=best, seq=bestseq), open(f'runs/b5_{cut}_{int(YG)}_{seed}{tag}.json', 'w'))
