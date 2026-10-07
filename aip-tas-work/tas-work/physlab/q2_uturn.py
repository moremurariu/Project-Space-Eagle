from labx import *
RT0 = 717
st = state_after('../kog_pregren_992.txt', RT0 + 68)
print('start', fmt(st))
L = read_inputs('../kog_pregren_992.txt')
N = 48
def gate(S, X=700):
    for i, s in enumerate(S):
        if s['frz']: return None, i
        if s['x'] >= X:
            p = S[i - 1]['x']; return RT0 + i + (X - p) / (s['x'] - p), i
    return None, len(S)
cands, meta = [], []
for f1 in (718, 720):
    for a1 in range(-90, 91, 10):
        for r1 in (725, 727, 729, 731, 733):
            for a2 in range(-80, -9, 10):
                for h2 in (2, 5):
                    for a3 in (-60, -75, None):
                        seq = []
                        f2 = r1 + 2
                        for rt in range(RT0 + 1, RT0 + 1 + N):
                            if f1 <= rt <= r1: c = (1, 0, 1, 0) + aim(a1) + (-1,)
                            elif f2 <= rt < f2 + h2: c = (1, 0, 1, 0) + aim(a2) + (-1,)
                            elif a3 is not None and rt >= f2 + h2 + 1 and (rt - f2 - h2) % 2 == 0: c = (1, 0, 1, 0) + aim(a3) + (-1,)
                            else: c = (1, 0, 0, 0, 0, 1000, -1)
                            seq.append(c)
                        cands.append(seq); meta.append((f1, a1, r1, a2, h2, a3))
print(len(cands), flush=True)
B = batch(st, cands)
base = run(os.path.abspath('tmp/empty.txt'), L)
bS = [s for s in base if RT0 < s['rt'] <= RT0 + N]
tb, ib = gate(bS); print('base: x>=700 at', tb, fmt(bS[ib]))
res = []
for m, S in zip(meta, B):
    t, i = gate(S)
    if t is None: continue
    landed = [s['rt'] if s['rt'] >= 0 else RT0 + 1 + k for k, s in enumerate(S[:i]) if abs(s['vy']) < 1e-6 and s['gr']]
    res.append((t - 0.02 * E(S[i]), t, m, E(S[i]), bool(landed), S[i]))
res.sort(key=lambda r: r[0])
print('alive', len(res), 'non-landing', sum(1 for r in res if not r[4]))
for r in res[:8]: print(f"J {r[0]:.2f} t {r[1]:.2f} E {r[3]:.0f} landed {r[4]} params {r[2]} {fmt(r[5])}")
nl = [r for r in res if not r[4]]
print('best non-landing:')
for r in nl[:8]: print(f"J {r[0]:.2f} t {r[1]:.2f} E {r[3]:.0f} params {r[2]} {fmt(r[5])}")
import pickle; pickle.dump([(r[0], r[1], r[2], r[3], r[4]) for r in res[:200]], open('runs_q2/uturn_scan.pkl', 'wb'))
