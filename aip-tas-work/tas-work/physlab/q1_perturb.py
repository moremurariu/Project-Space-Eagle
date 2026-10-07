import sys, pickle
from q1lib import *
from seqopt2 import *
import q1lib
seed = int(sys.argv[1]); NS = int(sys.argv[2])
rng = random.Random(seed)
N0 = 344; R0 = 277          # first input = rt 277
q1lib.RT0 = R0
N = 341 - R0 + 1
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
pulses = [(328, 330, 147.5), (331, 332, 151.3), (333, 334, 153.0), (335, 336, 154.7), (337, 339, 156.2), (340, 342, 158.2)]
def r(a, b): return rng.randint(a, b)
cands = []
for k in range(NS):
    segs = []
    a = 278 + r(-1, 1); b = 281 + r(-2, 2); segs.append((a, max(a + 1, b), 25.3 + rng.uniform(-4, 4)))
    a = max(segs[-1][1] + 1, 282 + r(-2, 2)); b = 290 + r(-3, 3); segs.append((a, max(a + 1, b), -106.9 + rng.uniform(-6, 6)))
    p = segs[-1][1] + 1
    for base_ang in (-136.5, -151.9):
        if rng.random() < 0.8:
            a = p + r(0, 2); L = r(1, 3); segs.append((a, a + L, base_ang + rng.uniform(-10, 10))); p = a + L + 1
    F = max(p, r(293, 299)); R = r(322, 330)
    segs.append((F, R, rng.uniform(48, 68)))
    sh = R - 327
    for (x, y, ang) in pulses:
        if x + sh > R + 1: segs.append((x + sh, y + sh, ang + rng.uniform(-2, 2)))
    dirs = [1] * N
    for _ in range(r(0, 3)):
        t = r(296, 327); dirs[t - R0] = rng.choice([0, -1])
    if rng.random() < 0.5: dirs[297 - R0] = 0
    jump = 284 + r(-3, 3)
    segs = [(x - R0, y - R0, ang) for x, y, ang in segs]
    cands.append((dirs, segs, [jump - R0]))
J, B = evalf(cands)
order = sorted(range(len(J)), key=lambda i: J[i])
print('alive', sum(1 for x in J if x < 900), 'of', len(J), flush=True)
for k in order[:8]:
    t, i = cross(B[k], 450, 9312, 9472)
    print(f'J {J[k]:.2f} t450 {t} {fmt(B[k][i])} jump {cands[k][2][0]+R0} segs {[(a+R0, b+R0, round(c, 1)) for a, b, c in cands[k][1][:5]]}', flush=True)
pickle.dump([(J[k], cands[k]) for k in order[:50]], open(f'runs_q1/perturb_{seed}.pkl', 'wb'))
for rr, k in enumerate(order[:3]):
    Jh, cur, S = hill3(evalf, cands[k], N, iters=80, pop=300, rng=random.Random(rr), first_free=0, log=False, patience=20)
    t, i = cross(S, 450, 9312, 9472)
    print(f'HILL {rr}: J {Jh:.3f} t450 {t} {fmt(S[i]) if t else ""}', flush=True)
    with open(f'runs_q1/perturb_{seed}_h{rr}.txt', 'w') as f:
        for c in to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
