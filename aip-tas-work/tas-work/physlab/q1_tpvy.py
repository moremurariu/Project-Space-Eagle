import sys
from q1lib import *
from seqopt2 import *
import q1lib
x0, y0, vx, vy = map(float, sys.argv[1:5]); RT = int(sys.argv[5])
q1lib.RT0 = RT + 1
N = 46
st = dict(x=x0, y=y0, vx=vx, vy=vy, jumped=2)
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None:
        alive = next((k for k, s in enumerate(S) if s['frz']), len(S))
        return 1000 - alive - max(s['x'] for s in S[:alive + 1]) / 50
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
best = None
for src, off in [('runs_q1/A_turn357.txt_0.txt', RT - 296)]:
    cand = from_inputs3(read_inputs(src)[off:off + N])
    cand = (cand[0] + [1] * (N - len(cand[0])), cand[1], [])
    for sd in range(3):
        J, cur, S = hill3(evalf, cand, N, iters=80, pop=300, rng=random.Random(sd), first_free=1, log=False, patience=20, nojump=True)
        if best is None or J < best[0]: best = (J, cur, S)
J, cur, S = best
t, i = cross(S, 450, 9312, 9472)
print(f'tp {x0} {y0} v {vx} {vy} at rt {RT}: J {J:.3f} t450 {t} {fmt(S[i]) if t else ""}', flush=True)
with open(f'runs_q1/tp_{x0:.0f}_{y0:.0f}_{vx:.2f}_{vy:.2f}.txt', 'w') as f:
    for c in to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
