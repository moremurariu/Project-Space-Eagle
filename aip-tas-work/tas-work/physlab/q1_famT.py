import sys
from q1lib import *
rng = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 1)
NS = int(sys.argv[2]) if len(sys.argv) > 2 else 20000
N = 50
s296 = state_after('../kog_pregren_992.txt', 364)
P = Problem(s296, N, make_obj(600, 0.0, 9312, 9472))
cands = []
for k in range(NS):
    f1 = rng.randint(1, 10); a1 = rng.uniform(15, 89); r1 = rng.randint(f1 + 2, 42)
    s1 = rng.randint(0, 40); s2 = rng.randint(s1, 45); dmid = rng.choice([-1, 0])
    d_end = rng.choice([-1, 0, 1])
    dirs = [1 if i < s1 else (dmid if i < s2 else d_end) for i in range(N)]
    segs = [(f1, r1, a1)]
    if rng.random() < 0.8:
        f2 = rng.randint(r1 + 1, r1 + 12); a2 = rng.uniform(90, 240); r2 = f2 + rng.randint(1, 15)
        segs.append((f2, r2, a2))
    cands.append((dirs, segs))
t0 = time.time()
J, B = P.evaluate(cands)
print('evals', len(J), 'sec', time.time() - t0)
order = sorted(range(len(J)), key=lambda i: J[i])
for k in order[:25]:
    S = B[k]; t, i = cross(S, 600, 9312, 9472); t4, i4 = cross(S, 450, 9312, 9472)
    print(f'J {J[k]:.2f} t450 {t4} t600 {t:.2f} {fmt(S[i])} segs {[(a,b,round(c,1)) for a,b,c in cands[k][1]]} dirs {"".join("+0-"[1-x] for x in cands[k][0][:45])}')
import pickle
pickle.dump([(J[k], cands[k]) for k in order[:200]], open(f'runs_q1/famT_{sys.argv[1] if len(sys.argv)>1 else 1}.pkl', 'wb'))
