# Climb optimiser scored by simulated double kicks (physlab2 c3_climb adapted to the new prefix + exact aims).
# usage: c6_climb.py PREFIX INITJSON N ITERS SEED [XF] ; env FROZEN (ticks not mutated), NMAX, FIXM, TAG, XG
import sys, math, time, json, random, os
from xl import *
from hc5 import *
from gmodel import face_hits, FACE_X
pre, initf, N, iters, seed = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
XF = float(sys.argv[6]) if len(sys.argv) > 6 else 5900
TAIL = int(os.environ.get('TAIL', '18'))
XG = float(os.environ.get('XG', '5671'))
VW = float(os.environ.get('VW', '0.05'))
ARCH = {}
d = json.load(open(initf)); cut = d['cut']
x = XL(); x.load(pre, cut + 68); x.save(0)
init = [norm(c) for c in (d.get('base') or d['seq'])]
init = (init + [[-1, 0, 1, 0, 250.0, None]] * N)[:N]
for c in init:
    c[3] = 0
ZONE = (5281, 5312, 1912, 1962)
FIXM = os.environ.get('FIXM')
NMAX = int(os.environ.get('NMAX', '35'))
FROZEN = int(os.environ.get('FROZEN', '0'))
TAG = os.environ.get('TAG', '')
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
    s = [norm(c) for c in seq[:e + 1]]
    if s[m][2] and (m == 0 or not s[m - 1][2]):
        return None
    s[m][3] = 1; setaim(s, m, a1)
    if s[e][2] and not s[e - 1][2]:
        s[e][2] = 0
    s[e][3] = 1; setaim(s, e, a2)
    return s + [[1, 0, 0, 0, 0.0, None] for _ in range(TAIL)]

def evaluate(cands):
    R = x.runmany([tuples(c) for c in cands], restore=0)
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
    info = {}
    if not jobs:
        return sc, info
    R2 = x.runmany([tuples(b) for _, _, b in jobs], restore=0)
    for (ci, e, b), S2 in zip(jobs, R2):
        if any(s['frz'] for s in S2):
            continue
        tg = None
        for k in range(e, len(S2)):
            if S2[k]['x'] >= XG:
                p = S2[k - 1]
                tg = p['rt'] + (XG - p['x']) / (S2[k]['x'] - p['x'])
                break
        if tg is None:
            continue
        vx = S2[e]['vx']
        v = tg - VW * vx
        ek = S2[e]['rt']
        if ek not in ARCH or ARCH[ek][0] > v:
            ARCH[ek] = (v, tg, vx, S2[e]['vy'], b)
        if sc[ci] is None or v < sc[ci]:
            sc[ci] = v
            info[ci] = (b, S2)
    return sc, info

rng = random.Random(seed)
sc, info = evaluate([init])
best = sc[0]; bestseq = init; bestinfo = info.get(0)
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(100):
        m_ = bestseq
        for _ in range(rng.choice((1, 1, 2, 3))):
            m_ = mutate(m_, rng, None, 0, False, lo=FROZEN)
        cands.append(m_)
    sc, info = evaluate(cands)
    for i, v in enumerate(sc):
        if v is not None and (best is None or v <= best):
            best, bestseq, bestinfo = v, cands[i], info[i]
    if it % 10 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
if bestinfo:
    b, S2 = bestinfo
    for c, s in zip(b, S2):
        print(line(c).ljust(30), '|', fmt(s))
    json.dump(dict(cut=cut, best=best, base=bestseq, seq=b), open(f'runs/c6_{cut}_{seed}{TAG}.json', 'w'))
for ek in sorted(ARCH):
    v, tg, vx, vy, b = ARCH[ek]
    print('ARCH exit', ek, 'score %.3f t%d %.3f v %.2f %.2f' % (v, XG, tg, vx, vy))
json.dump({str(k): dict(score=v[0], tg=v[1], vx=v[2], vy=v[3], seq=v[4]) for k, v in ARCH.items()}, open(f'runs/c6arch_{cut}_{seed}{TAG}.json', 'w'))
