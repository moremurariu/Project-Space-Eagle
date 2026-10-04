from labx import *
import itertools, random
RT0 = 731
st = state_after('../kog_pregren_992.txt', RT0 + 68)
N = 34
def gate(S, X=700):
    for i, s in enumerate(S):
        if s['frz']: return None, i
        if s['x'] >= X:
            p = S[i - 1]['x']; return RT0 + i + (X - p) / (s['x'] - p), i
    return None, len(S)
cands, meta = [], []
for f1 in (732, 733, 734):
    for a1 in range(-90, -5, 5):
        for h1 in (1, 2, 3, 4):
            for a2 in list(range(-90, -5, 10)) + [None]:
                for a3 in (-50, -60, -70, -80, None):
                    seq = []
                    f2 = f1 + h1 + 1
                    for rt in range(RT0 + 1, RT0 + 1 + N):
                        if f1 <= rt < f1 + h1: c = (1, 0, 1, 0) + aim(a1) + (-1,)
                        elif a2 is not None and f2 <= rt < f2 + 2: c = (1, 0, 1, 0) + aim(a2) + (-1,)
                        elif a3 is not None and rt >= f2 + 3 and (rt - f2) % 2 == 1: c = (1, 0, 1, 0) + aim(a3) + (-1,)
                        else: c = (1, 0, 0, 0, 0, 1000, -1)
                        seq.append(c)
                    cands.append(seq); meta.append((f1, a1, h1, a2, a3))
print(len(cands), flush=True)
B = batch(st, cands)
base = run(os.path.abspath('tmp/empty.txt'), read_inputs('../kog_pregren_992.txt'))
bS = [s for s in base if RT0 < s['rt'] <= RT0 + N]
tb, ib = gate(bS); print('base: x>=700 at', tb, fmt(bS[ib]))
res = []
for m, S in zip(meta, B):
    t, i = gate(S)
    if t is None: continue
    landed = any(abs(s['vy']) < 1e-6 and s['gr'] for s in S[:i])
    res.append((t - 0.02 * E(S[i]), t, m, E(S[i]), landed, S[i]))
res.sort(key=lambda r: r[0])
print('alive', len(res))
for r in res[:10]: print(f"J {r[0]:.2f} t {r[1]:.2f} E {r[3]:.0f} landed {r[4]} params {r[2]} {fmt(r[5])}")
nl = [r for r in res if not r[4]]
print('best non-landing:'); 
for r in nl[:5]: print(f"J {r[0]:.2f} t {r[1]:.2f} E {r[3]:.0f} params {r[2]} {fmt(r[5])}")
