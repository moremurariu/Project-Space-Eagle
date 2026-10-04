from q1lib import *
from seqopt2 import *
import itertools
s296 = state_after('../kog_pregren_992.txt', 364)
N = 46
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None: return 1000
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - 0.3 * S[i]['vy']
cands, meta = [], []
for d0 in (0, 1):
  for a in (62, 64.2, 66):
    for T1 in range(316, 328):
        for T2 in range(T1 + 1, T1 + 7):
            for R in range(T1, 331, 2):
                for pa in (147, 152, 157):
                    dirs = [d0] + [1 if (rt < T1 or rt >= T2) else -1 for rt in range(298, 297 + N)]
                    segs = [(1, R - 297 + 1, a)]
                    p = R - 297 + 2
                    while p < N - 1:
                        segs.append((p, p + 2, pa)); p += 3
                    cands.append((dirs, segs, [])); meta.append((d0, a, T1, T2, R, pa))
print(len(cands), flush=True)
B = batch(s296, [to_inputs3(d, s, j, N) for d, s, j in cands])
res = sorted((obj(S), m, k) for k, (m, S) in enumerate(zip(meta, B)))
for J, m, k in res[:12]:
    S = B[k]; t, i = cross(S, 450, 9312, 9472)
    print(f'J {J:.2f} dir0 {m[0]} aim {m[1]} dir-1 rt {m[2]}..{m[3]-1} hook until {m[4]} pulses {m[5]}: t450 {t} {fmt(S[i]) if t else ""}')
import pickle; pickle.dump([(J, cands[k]) for J, m, k in res[:20]], open('runs_q1/corner.pkl', 'wb'))
