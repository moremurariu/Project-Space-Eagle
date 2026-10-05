# Local polish of a full input file: hill-climb the inputs from rt CUT+1 to the end (incl. the point-blank shot),
# score = fractional rt at x = XG (default 5671) - VW * vx two ticks after the exit; alive throughout.
# usage: p7.py FILE CUT ITERS SEED OUT [NFIRE]   env XG, VW, ENDRT (extend/trim to this rt with dir-1 lines)
import sys, os, random, time, json
from xl import *
from hc5 import *
f, cut, iters, seed, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), sys.argv[5]
NF = int(sys.argv[6]) if len(sys.argv) > 6 else 1
XG = float(os.environ.get('XG', '5671')); VW = float(os.environ.get('VW', '0.03'))
L = open(f).readlines()
ENDRT = int(os.environ.get('ENDRT', str(len(L) - 68)))
seq = from_lines(L[cut + 68:])
while len(seq) < ENDRT - cut:
    seq.append([1, 0, 0, 0, 0.0, [1000, 0]])
seq = seq[:ENDRT - cut]
# keep a pending pre-fire: shots before the cut stay in the prefix; count shots after the cut
nf0 = sum(1 for c in seq if c[3])
x = XL(); x.load(f, cut + 68); x.save(0)
def score(S):
    if any(s['frz'] for s in S):
        return None
    for a, b in zip(S, S[1:]):
        if b['x'] >= XG > a['x']:
            tg = a['rt'] + (XG - a['x']) / (b['x'] - a['x'])
            i = S.index(b)
            ex = next((k for k, s in enumerate(S) if s['vx'] > 20 and s['rt'] > 1005), None)
            if ex is None:
                return None
            vx = S[min(ex + 1, len(S) - 1)]['vx']
            return tg - VW * vx
    return None
rng = random.Random(seed)
best = score(x.runmany([tuples(seq)], restore=0)[0]); bestseq = seq
print('init', best, flush=True)
t0 = time.time()
for it in range(iters):
    cands = []
    for _ in range(120):
        m = bestseq
        for _ in range(rng.choice((1, 1, 2, 3))):
            m = mutate(m, rng, None, max(NF, nf0), True)
        cands.append(m)
    R = x.runmany([tuples(c) for c in cands], restore=0)
    for c, S in zip(cands, R):
        v = score(S)
        if v is not None and (best is None or v <= best):
            best, bestseq = v, c
    if it % 50 == 0:
        print(it, best, f'{time.time()-t0:.0f}s', flush=True)
print('BEST', best)
with open(out, 'w') as g:
    g.writelines(L[:cut + 68])
    for c in bestseq:
        g.write(line(norm(c)) + '\n')
