import sys
from q1lib import *
from seqopt2 import *
import q1lib
# rt-296 state of the 992 run, but with the air jump available (teleport experiment): does a jump used as a vertical brake help?
q1lib.RT0 = 297
N = 46
st = dict(x=8883, y=471, vx=5907 / 256, vy=-5874 / 256, jumped=0)
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None:
        alive = next((k for k, s in enumerate(S) if s['frz']), len(S))
        return 1000 - alive
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
def evalf(cands):
    B = batch(st, [to_inputs3(d, s, j, N) for d, s, j in cands])
    return [obj(S) for S in B], B
A = read_inputs('runs_q1/A_turn357.txt_0.txt')[:N]
d0, s0, _ = from_inputs3(A)
# scan: jump at tick j (0..20), and pillar hook fired at f with angle a held to R
cands = []
for j in range(0, 22):
    for (f, a, R) in [(1, 64.2, 30), (1, 60, 30), (1, 56, 30), (1, 50, 28), (2, 64, 30), (4, 60, 30)] + [(f, a, R) for f in (1, 3, 6, 9) for a in (40, 50, 60, 70) for R in (24, 27, 30)]:
        segs = [(f, R, a)] + [s for s in s0 if s[0] > R + 1]
        cands.append((d0, segs, [j]))
J, B = evalf(cands)
order = sorted(range(len(J)), key=lambda i: J[i])
for k in order[:12]:
    t, i = cross(B[k], 450, 9312, 9472)
    print(f'J {J[k]:.2f} jump@rt {297 + cands[k][2][0]} hook {cands[k][1][0]} t450 {t} {fmt(B[k][i]) if t else ""}')
best = []
for r, k in enumerate(order[:3]):
    Jh, cur, S = hill3(evalf, cands[k], N, iters=60, pop=300, rng=random.Random(r), first_free=1, log=False, patience=15)
    t, i = cross(S, 450, 9312, 9472)
    print(f'HILL {r}: J {Jh:.3f} t450 {t} {fmt(S[i]) if t else ""} jumps {[297 + x for x in cur[2]]} segs {[(297+a, 297+b, round(c, 1)) for a, b, c in cur[1]][:3]}', flush=True)
    with open(f'runs_q1/jumpbrake_h{r}.txt', 'w') as f:
        for c in to_inputs3(*cur, N): f.write(' '.join(map(str, c)) + '\n')
