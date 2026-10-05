# Stage 2+3: hill-climb the climb (no shots in the mutation) and score each base by simulated double kicks:
# analytic pre-fire options (gmodel) -> simulate pre-fire + point-blank + a fixed post-kick tail (dir 1, no hook),
# score = estimated rt at x = XF (rt_end + (XF - x)/vx), alive through the tail.
# usage: c3_climb.py CUTLINES INITJSON N ITERS SEED [XF]
import sys, math, time, json, random
from xl import *
from hc import *
from gmodel import face_hits, FACE_X
cut = int(sys.argv[1]); initf = sys.argv[2]; N = int(sys.argv[3]); iters = int(sys.argv[4]); seed = int(sys.argv[5])
XF = float(sys.argv[6]) if len(sys.argv) > 6 else 5900
TAIL = 14
x = XL(); x.load('pre978w.txt', cut); x.save(0)
d = json.load(open(initf))
init = (d['seq'] + [[-1, 0, 1, 0, 250.0]] * N)[:N]
import os
KEEP = int(os.environ.get('KEEPFIRE', '0'))   # keep the init's shots in the first KEEP ticks (e.g. a brake shot)
for i, c in enumerate(init):
    if i >= KEEP:
        c[3] = 0
ZONE = (5281, 5312, 1912, 1962)

FIXM = os.environ.get('FIXM')      # only this fire step (seq index) for the pre-fire
NMAX = int(os.environ.get('NMAX', '35'))
FROZEN = int(os.environ.get('FROZEN', '0'))  # don't mutate the first FROZEN ticks
def options(S, kmax=4):
    out = []
    for m in range(1, len(S)):
        if FIXM is not None and m != int(FIXM):
            continue
        p = S[m - 1]
        if p['reload'] != 0 or p['gren'] != 2 or p['frz'] or not (2200 <= p['y'] <= 2340):
            continue
        for n in range(26, NMAX + 1):
            e = m + n - 1
            if e >= len(S) - 1:
                break
            t = S[e - 1]
            if not (ZONE[0] <= t['x'] <= ZONE[1] and ZONE[2] <= t['y'] <= ZONE[3]):
                continue
            hits = [(a, yc) for a, yc in face_hits(p['x'], p['y'], n) if math.hypot(t['x'] - FACE_X, t['y'] - yc) <= 47]
            if hits:
                a, yc = hits[len(hits) // 2]
                out.append((e, m, n, a, yc))
    out.sort()
    return out[:kmax]

def build(seq, m, a1, e, a2):
    s = [list(c) for c in seq[:e + 1]]
    s[m][3] = 1; s[m][4] = a1
    if s[m][2] and (m == 0 or not s[m - 1][2]):
        return None  # shot on a hook-launch tick would re-aim the hook
    s[e][3] = 1; s[e][4] = a2
    if s[e][2] and not s[e - 1][2]:
        s[e][2] = 0
    tail = [[1, 0, 0, 0, 0.0]] * TAIL
    return s + tail

def evaluate(cands):
    R = x.runmany([seq_tuples(c) for c in cands], restore=0)
    jobs = []
    for ci, (c, S) in enumerate(zip(cands, R)):
        if any(s['frz'] for s in S):
            continue
        for (e, m, n, a, yc) in options(S):
            t = S[e - 1]
            base_a2 = math.degrees(math.atan2(min(max(t['y'] - 4, 1922), 1950) - t['y'], FACE_X - t['x']))
            for da in (-6, 0, 6):
                b = build(c, m, a, e, base_a2 + da)
                if b is not None:
                    jobs.append((ci, e, b))
    sc = [None] * len(cands)
    if not jobs:
        return sc, {}
    R2 = x.runmany([seq_tuples(b) for _, _, b in jobs], restore=0)
    info = {}
    for (ci, e, b), S2 in zip(jobs, R2):
        if any(s['frz'] for s in S2):
            continue
        end = S2[-1]
        if end['vx'] <= 1:
            continue
        # estimated time at x = XF from the end of the tail
        v = end['rt'] + (XF - end['x']) / end['vx']
        if sc[ci] is None or v < sc[ci]:
            sc[ci] = v
            info[ci] = (b, S2)
    return sc, info

rng = random.Random(seed)
best = None; bestseq = init; bestinfo = None
sc, info = evaluate([init])
best = sc[0]; bestinfo = info.get(0)
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(100):
        m_ = bestseq
        for _ in range(rng.choice((1, 1, 2, 3))):
            m_ = mutate(m_, rng, None, 0, False)
        for i in range(FROZEN):
            m_[i] = list(init[i])
        for i in range(KEEP):
            m_[i][3] = init[i][3]
            if init[i][3]:
                m_[i][4] = init[i][4]
        cands.append(m_)
    sc, info = evaluate(cands)
    for i, v in enumerate(sc):
        if v is not None and (best is None or v <= best):
            best, bestseq, bestinfo = v, cands[i], info[i]
    if it % 10 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
b, S2 = bestinfo
for c, s in zip(b, S2):
    print(c[0], c[1], c[2], c[3], f'{c[4]:.3f}', '|', fmt(s))
json.dump(dict(cut=cut, best=best, seq=bestseq, full=b), open(f'runs_b/c3_{cut}_{seed}.json', 'w'))
