# Stage 2: hill-climb the whole post-cut sequence so that the tee is in the exit zone exactly 25 ticks after a
# possible pre-fire tick F1 (tee y in [YLO, YHI], reload 0, has grenade). Score = (F1 + 25, tie).
# usage: c1_climb.py CUTLINES INITJSON N ITERS SEED [zone x0 x1 y0 y1]
import sys, math, time, json
from xl import *
from hc import *
cut = int(sys.argv[1]); initf = sys.argv[2]; N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
Z = [float(v) for v in sys.argv[6:10]] if len(sys.argv) > 9 else [5282, 5302, 1925, 1965]
YLO, YHI = 2250, 2300
x = XL(); x.load('pre978w.txt', cut); x.save(0)
d = json.load(open(initf))
init = d['seq']
if d.get('cut', cut) != cut:
    raise SystemExit('cut mismatch')
# extend with a hook climb toward the L-arm underside
init = (init + [[-1, 0, 0, 0, 250.0]] + [[-1, 0, 1, 0, 250.0]] * N)[:N]
for c in init:
    c[3] = 0  # no shots in the base
def zone(s):
    return Z[0] <= s['x'] <= Z[1] and Z[2] <= s['y'] <= Z[3]
def score(S):
    best = None
    for i, s in enumerate(S):
        if s['frz']:
            return None
        if YLO <= s['y'] <= YHI and s['reload'] == 0 and s['gren'] == 2:
            j = i + 25
            if j < len(S) and zone(S[j]) and not any(t['frz'] for t in S[i:j + 3]):
                # tie: prefer upward speed ~ -10 and centre of the zone
                sj = S[j]
                tie = abs(sj['vy'] + 10) * 0.1 + abs(sj['y'] - 1945) * 0.01
                cand = (S[j]['rt'], tie)
                if best is None or cand < best:
                    best = cand
    if best is None:
        # shaping: distance to the zone 25 ticks after the first pre-fire-capable tick
        for i, s in enumerate(S):
            if s['y'] <= YHI and s['gren'] == 2:
                j = min(i + 25, len(S) - 1)
                dz = math.hypot(max(0, Z[0] - S[j]['x'], S[j]['x'] - Z[1]), max(0, Z[2] - S[j]['y'], S[j]['y'] - Z[3]))
                return (S[j]['rt'] + 100 + dz * 0.1, 0)
        return (9999, 0)
    return best
t = time.time()
best, seq = climb(x, 0, init, score, iters=iters, lam=120, seed=seed, allow_fire=False,
                  log=lambda it, b: print(it, b, f'{time.time()-t:.0f}s', flush=True) if it % 50 == 0 else None)
print('BEST', best)
S = x.runmany([seq_tuples(seq)], restore=0)[0]
for c, s in zip(seq, S):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.2f}', '|', fmt(s), 'g', s['gren'], 'hk', s['hx'], s['hy'])
json.dump(dict(cut=cut, best=best, seq=seq), open(f'runs_b/c1_{cut}_{seed}.json', 'w'))
