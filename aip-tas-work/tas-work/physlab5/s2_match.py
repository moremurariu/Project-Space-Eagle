# s2_match.py PREFIX CUT S1JSON:K OLDFILE OLDG NEWG ITERS SEED TAG
# hill-climb the landing part (rt CUT+1..NEWG+1) so that the state at NEWG+1 matches OLDFILE's state at OLDG+1,
# with OLDFILE's inputs from rt OLDG+2 on appended (fixed). Score: fractional y=2290 gate if reached, else 100 + mismatch.
import sys, json, random, time
from xl import *
from hc5 import *
from hc import gate_score
pre, cut, s1, oldf, oldg, newg, iters, seed, tag = sys.argv[1], int(sys.argv[2]), sys.argv[3], sys.argv[4], int(sys.argv[5]), int(sys.argv[6]), int(sys.argv[7]), int(sys.argv[8]), sys.argv[9]
f, k = s1.split(':')
head = [norm(c) for c in json.load(open(f))[int(k)]['seq']][:newg + 1 - cut]
while len(head) < newg + 1 - cut:
    head.append([1, 0, 0, 0, 0.0, None])
OL = open(oldf).readlines()
tail = from_lines(OL[oldg + 1 + 68:])[:1000 - (oldg + 1)]
xo = XL(); xo.load(oldf, oldg + 1 + 68); T = xo.state(); xo.close()
print('target', fmt(T))
x = XL(); x.load(pre, cut + 68); x.save(0)
g = gate_score(ygate=2290, xmax=5376, hold=6, cond=lambda s: s['gren'] == 2 and s['reload'] == 0 and s['vy'] <= -12, tie=lambda s: s['y'])
def score(S):
    r = g(S)
    if r is not None:
        i = next(j for j, s in enumerate(S) if s['rt'] == r[0]); p = S[i - 1]
        return p['rt'] + (p['y'] - 2290) / (p['y'] - S[i]['y'])
    s = S[newg + 1 - cut - 1]
    if any(t['frz'] for t in S[:newg + 1 - cut]):
        return None
    m = abs(s['x'] - T['x']) + abs(s['y'] - T['y']) + 2 * abs(s['vx'] - T['vx']) + 2 * abs(s['vy'] - T['vy']) + (0 if s['hook'] == T['hook'] else 5)
    if s['jumped'] & 2:
        m += 50
    return 100 + m
rng = random.Random(seed)
best = score(x.runmany([tuples(head + tail)], restore=0)[0]); bh = head
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(150):
        m = bh
        for _ in range(rng.choice((1, 1, 2, 3))):
            m = mutate(m, rng, None, 0, False)
        cands.append(m)
    R = x.runmany([tuples(c + tail) for c in cands], restore=0)
    for c, S in zip(cands, R):
        v = score(S)
        if v is not None and (best is None or v <= best):
            best, bh = v, c
    if it % 50 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
json.dump(dict(cut=cut, best=best, seq=bh + tail), open(f'runs/s2_{cut}_{seed}{tag}.json', 'w'))
S = x.runmany([tuples(bh + tail)], restore=0)[0]
for c, s in zip(bh + tail, S):
    if s['rt'] <= 995:
        print(line(c).ljust(28), '|', fmt(s), 'g', s['gren'])
