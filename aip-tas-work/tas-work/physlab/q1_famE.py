import sys, pickle
from q1lib import *
from seqopt2 import *
import q1lib
seed = int(sys.argv[1]); NS = int(sys.argv[2]); N0 = 344
rng = random.Random(seed)
N = 341 - (N0 - 68) + 1
q1lib.RT0 = N0 - 68 + 1
st = state_after('../kog_pregren_992.txt', N0)
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
cands = []
for k in range(NS):
    j = rng.randint(0, 16)
    fA = rng.randint(0, 12); aA = rng.uniform(-160, -60); LA = rng.randint(1, 14)
    fB = rng.randint(fA + LA + 1, fA + LA + 10); aB = rng.uniform(25, 80); LB = rng.randint(12, 40)
    segs = [(fA, fA + LA, aA), (fB, fB + LB, aB)]
    p = fB + LB + 1
    pa = rng.uniform(140, 160)
    while p < N - 2:
        L = rng.choice([1, 2, 2, 3])
        segs.append((p, p + L, pa)); pa += rng.uniform(0, 3)
        p += L + 1
    dirs = [1] * N
    for _ in range(rng.randint(0, 3)):
        a = rng.randint(0, N - 1); L = rng.randint(1, 4); v = rng.choice([-1, 0])
        for q in range(a, min(N, a + L)): dirs[q] = v
    cands.append((dirs, segs, [j]))
J, B = evalf(cands)
order = sorted(range(len(J)), key=lambda i: J[i])
print('ok', sum(1 for x in J if x < 900), 'of', len(J))
for k in order[:10]:
    t, i = cross(B[k], 450, 9312, 9472)
    print(f'J {J[k]:.2f} t450 {t} {fmt(B[k][i])} jump {cands[k][2]} segs {[(a, b, round(c, 1)) for a, b, c in cands[k][1][:2]]}')
pickle.dump([(J[k], cands[k]) for k in order[:100]], open(f'runs_q1/famE_{seed}.pkl', 'wb'))
# hill-climb the best 4
for r, k in enumerate(order[:4]):
    Jh, cur, S = hill3(evalf, cands[k], N, iters=60, pop=250, rng=random.Random(r), first_free=0, log=False, patience=15)
    t, i = cross(S, 450, 9312, 9472)
    print(f'HILL {r}: J {Jh:.3f} t450 {t} {fmt(S[i]) if t else ""}', flush=True)
    with open(f'runs_q1/famE_{seed}_h{r}.txt', 'w') as f:
        for c in to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
