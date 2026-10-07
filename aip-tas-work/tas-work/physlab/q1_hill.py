import sys, json
from q1lib import *
seed = int(sys.argv[1]) if len(sys.argv) > 1 else 1
iters = int(sys.argv[2]) if len(sys.argv) > 2 else 40
lam = float(sys.argv[3]) if len(sys.argv) > 3 else 0.03
init = sys.argv[4] if len(sys.argv) > 4 else 'base'
N = int(os.environ.get("NT", "80"))
s296 = state_after('../kog_pregren_992.txt', 364)
if init == 'base':
    base = read_inputs('../kog_pregren_992.txt')[364:364 + N]
    base[0] = (base[0][0], 0, 0, 0, base[0][4], base[0][5], -1)
else:
    base = read_inputs(init)[:N]
    while len(base) < N: base.append((0, 0, 0, 0, 0, -1000, -1))
d, s = from_inputs(base)
P = Problem(s296, N, make_obj(1100, lam, 9100, 9248))
t0 = time.time()
J, cur, S = hill(P, d, s, iters=iters, pop=300, rng=random.Random(seed))
t, i = cross(S, 1100, 9100, 9248)
print('final J', J, 't1100', t, fmt(S[i]), 'evals', P.evals, 'sec', time.time() - t0)
t450, i450 = cross(S, 450); t600, i600 = cross(S, 600)
print('t450', t450, fmt(S[i450])); print('t600', t600, fmt(S[i600]))
out = f'runs_q1/hill_s{seed}_l{lam}.txt'
os.makedirs('runs_q1', exist_ok=True)
with open(out, 'w') as f:
    for c in to_inputs(cur[0], cur[1], N)[:i + 1]:
        f.write(' '.join(map(str, c)) + '\n')
print('wrote', out)
