import sys
from q1lib import *
from seqopt2 import *
import q1lib
# from rt 276 (state after line 344): 992 inputs until rt X (inclusive), then release 1 tick, fire hook at angle a at X+2, hold to rt R, dir 1;
# then left pulses (from the stage-A best) in the shaft.
N0 = 344; RTs = N0 - 68
q1lib.RT0 = RTs + 1
N = 341 - RTs
st = state_after('../kog_pregren_992.txt', N0)
L = read_inputs('../kog_pregren_992.txt')
A = read_inputs('runs_q1/A_turn357.txt_0.txt')  # from rt 297
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
seqs, meta = [], []
for X in range(282, 297):
    for a in range(30, 81, 2):
        for R in range(318, 330, 2):
            seq = []
            for rt in range(RTs + 1, 342):
                if rt <= X:
                    c = L[rt + 67]   # input producing rt
                elif rt == X + 1:
                    c = (1, 0, 0, 0, 0, 1000, -1)
                elif rt <= R:
                    c = (1, 0, 1, 0) + aim(a) + (-1,)
                elif rt == R + 1:
                    c = (1, 0, 0, 0, 0, 1000, -1)
                else:
                    c = A[rt - 297] if rt - 297 < len(A) else (1, 0, 0, 0, 0, 1000, -1)
                    if rt == R + 2 and c[2]:
                        pass
                seq.append(c)
            seqs.append(seq); meta.append((X, a, R))
B = batch(st, seqs)
res = sorted((obj(S), m, S) for m, S in zip(meta, B))
for J, m, S in res[:15]:
    t, i = cross(S, 450, 9312, 9472)
    print(f'J {J:.2f} X {m[0]} aim {m[1]} R {m[2]} t450 {t} {fmt(S[i]) if t else ""}')
print('alive', sum(1 for r in res if r[0] < 900), 'of', len(res))
