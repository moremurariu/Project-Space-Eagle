import sys
from q1lib import *
from seqopt2 import *
import q1lib
N0 = 344; RTs = N0 - 68
q1lib.RT0 = RTs + 1
st = state_after('../kog_pregren_992.txt', N0)
L = read_inputs('../kog_pregren_992.txt')
A = read_inputs('runs_q1/A_turn357.txt_0.txt')  # from rt 297
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
src = sys.argv[1] if len(sys.argv) > 1 else '../kog_pregren_992.txt'
L = read_inputs(src)
seqs, meta = [], []
for X in range(283, 296):
    for F in range(X + 2, X + 7):
        for a in range(26, 84, 2):
            for R in (324, 326, 328):
                seq = []
                for rt in range(RTs + 1, 342):
                    if rt <= X: c = L[rt + 67]
                    elif rt < F: c = (1, 0, 0, 0, 0, 1000, -1)
                    elif rt <= R: c = (1, 0, 1, 0) + aim(a) + (-1,)
                    elif rt == R + 1: c = (1, 0, 0, 0, 0, 1000, -1)
                    else: c = A[rt - 297] if rt - 297 < len(A) else (1, 0, 0, 0, 0, 1000, -1)
                    seq.append(c)
                seqs.append(seq); meta.append((X, F, a, R))
B = batch(st, seqs)
res = sorted((obj(S), m, S) for m, S in zip(meta, B))
for J, m, S in res[:15]:
    t, i = cross(S, 450, 9312, 9472)
    print(f'J {J:.2f} keep-until {m[0]} fire {m[1]} aim {m[2]} R {m[3]} t450 {t} {fmt(S[i]) if t else ""}')
print('alive', sum(1 for r in res if r[0] < 900), 'of', len(res))
