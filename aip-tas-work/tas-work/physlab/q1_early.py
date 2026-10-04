import sys, pickle
from q1lib import *
from seqopt2 import *
import q1lib
seed = int(sys.argv[1]); iters = int(sys.argv[2]); src = sys.argv[3]; lamv = float(sys.argv[4]); N0 = int(sys.argv[5])
N = 341 - (N0 - 68) + 1   # simulate to rt 341
q1lib.RT0 = N0 - 68 + 1
st = state_after('../kog_pregren_992.txt', N0)
pre = os.path.abspath(f'tmp/pre{N0}.txt'); open(pre, 'w').writelines(open('../kog_pregren_992.txt').readlines()[:N0])
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - lamv * S[i]['vy']
def evalf(cands):
    seqs = [to_inputs3(d, s, j, N) for d, s, j in cands]
    B = batch(st, seqs)
    return [obj(S) for S in B], B
if src.endswith('.txt'):
    base = read_inputs(src)
    off = int(sys.argv[6]) if len(sys.argv) > 6 else 0
    base = base[off:off + N]
    while len(base) < N: base.append((1, 0, 0, 0, 0, 1000, -1))
    cand = from_inputs3(base)
(J0,), (S0,) = evalf([cand])
T = run(pre, to_inputs3(*cand, N))
same = all((a['x'], a['y'], a['vx'], a['vy']) == (b['x'], b['y'], b['vx'], b['vy']) for a, b in zip(S0, T))
print('seed J', J0, 'batch==run', same, flush=True)
J, cur, S = hill3(evalf, cand, N, iters=iters, pop=300, rng=random.Random(seed), first_free=0, log=False, patience=25)
t, i = cross(S, 450, 9312, 9472)
print(f'seed {seed} src {src} N0 {N0} J {J:.3f} t450 {t} {fmt(S[i]) if t else ""}')
print('segs', [(a, b, round(c, 2)) for a, b, c in cur[1]], 'jumps', cur[2], 'dirs', ''.join('+0-'[1 - x] for x in cur[0]))
out = f'runs_q1/E{N0}_{os.path.basename(src)}_{seed}.txt'
with open(out, 'w') as f:
    for c in to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
print('wrote', out)
