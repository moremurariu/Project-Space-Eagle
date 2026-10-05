# Single point-blank exit (no pre-fire): hill-climb the climb (no shots) from a cut; score = best over zone ticks of
# a simulated point-blank shot at the L-arm face (3 aims) + 14-tick dir-1 tail -> estimated rt at x = 5900.
# usage: c5_single.py CUT INITJSON N ITERS SEED
import sys, math, time, json, random
from xl import *
from hc import *
cut = int(sys.argv[1]); initf = sys.argv[2]; N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
TAIL = 14; XF = 5900
x = XL(); x.load('pre978w.txt', cut); x.save(0)
d = json.load(open(initf))
init = (d['seq'] + [[-1, 0, 1, 0, 250.0]] * N)[:N]
for c in init:
    c[3] = 0
ZONE = (5281, 5300, 1920, 1975)
def evaluate(cands):
    R = x.runmany([seq_tuples(c) for c in cands], restore=0)
    jobs = []
    for ci, (c, S) in enumerate(zip(cands, R)):
        if any(s['frz'] for s in S):
            continue
        k = 0
        for i in range(1, len(S) - 1):
            t = S[i - 1]
            if t['reload'] == 0 and t['gren'] == 2 and ZONE[0] <= t['x'] <= ZONE[1] and ZONE[2] <= t['y'] <= ZONE[3]:
                base = math.degrees(math.atan2(min(max(t['y'] - 4, 1922), 1950) - t['y'], 5248 - t['x']))
                for da in (-8, 0, 8):
                    b = [list(cc) for cc in c[:i + 1]]
                    b[i][3] = 1; b[i][4] = base + da
                    if b[i][2] and not b[i - 1][2]:
                        b[i][2] = 0
                    jobs.append((ci, b + [[1, 0, 0, 0, 0.0]] * TAIL))
                k += 1
                if k >= 3:
                    break
    sc = [None] * len(cands); info = {}
    if not jobs:
        return sc, info
    R2 = x.runmany([seq_tuples(b) for _, b in jobs], restore=0)
    for (ci, b), S2 in zip(jobs, R2):
        if any(s['frz'] for s in S2) or S2[-1]['vx'] <= 1:
            continue
        e = S2[-1]
        v = e['rt'] + (XF - e['x']) / e['vx']
        if sc[ci] is None or v < sc[ci]:
            sc[ci] = v; info[ci] = (b, S2)
    return sc, info
rng = random.Random(seed)
sc, info = evaluate([init]); best = sc[0]; bestseq = init; bestinfo = info.get(0)
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(100):
        m_ = bestseq
        for _ in range(rng.choice((1, 1, 2, 3))):
            m_ = mutate(m_, rng, None, 0, False)
        cands.append(m_)
    sc, info = evaluate(cands)
    for i, v in enumerate(sc):
        if v is not None and (best is None or v <= best):
            best, bestseq, bestinfo = v, cands[i], info[i]
    if it % 20 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
b, S2 = bestinfo
for c, s in zip(b, S2):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.3f}', '|', fmt(s))
json.dump(dict(cut=cut, best=best, seq=bestseq, full=b), open(f'runs_b/c5_{cut}_{seed}.json', 'w'))
