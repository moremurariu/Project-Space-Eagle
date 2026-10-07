import sys
from q1lib import *
from seqopt2 import *
import q1lib
N0 = 344; RTs = N0 - 68
q1lib.RT0 = RTs + 1
st = state_after('../kog_pregren_992.txt', N0)
src = sys.argv[1] if len(sys.argv) > 1 else '../kog_pregren_992.txt'
L = read_inputs(src)
A = read_inputs('runs_q1/A_turn357.txt_0.txt')  # from rt 297
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
seqs, meta = [], []
for X in range(286, 296):          # keep original inputs up to X
    for F in (X + 2,):
        for a in range(70, 121, 3):   # down hook
            for R1 in range(F + 3, F + 16, 2):
                for a2, R2 in ((64.2, 326), (60, 326), (56, 326), (66, 324)):
                    F2 = R1 + 2
                    seq = []
                    for rt in range(RTs + 1, 342):
                        if rt <= X: c = L[rt + 67]
                        elif rt < F: c = (1, 0, 0, 0, 0, 1000, -1)
                        elif rt <= R1: c = (1, 0, 1, 0) + aim(a) + (-1,)
                        elif rt < F2: c = (1, 0, 0, 0, 0, 1000, -1)
                        elif rt <= R2: c = (1, 0, 1, 0) + aim(a2) + (-1,)
                        elif rt == R2 + 1: c = (1, 0, 0, 0, 0, 1000, -1)
                        else: c = A[rt - 297]
                        seq.append(c)
                    seqs.append(seq); meta.append((X, F, a, R1, a2, R2))
print(len(seqs), flush=True)
B = batch(st, seqs)
res = sorted((obj(S), m, S) for m, S in zip(meta, B))
for J, m, S in res[:15]:
    t, i = cross(S, 450, 9312, 9472)
    print(f'J {J:.2f} keep {m[0]} down-hook fire {m[1]} aim {m[2]} until {m[3]}; pillar hook {m[4]} until {m[5]}: t450 {t} {fmt(S[i]) if t else ""}')
print('alive', sum(1 for r in res if r[0] < 900), 'of', len(res))
import pickle
pickle.dump([(r[0], r[1]) for r in res[:50]], open('runs_q1/down_scan.pkl', 'wb'))
