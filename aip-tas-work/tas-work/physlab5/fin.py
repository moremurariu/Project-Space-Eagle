# fin.py FILE K P OUT: exhaustive final approach: inputs for rt K+1..P-1 from a small action set, point-blank fired on
# step P (aim grid), then dir-1 tail; keep the original tail after the kick if better. Score: fractional t at x=5671.
import sys, itertools, math
from xl import *
f, K, P, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
L = open(f).readlines()
x = XL(); x.load(f, K + 68); x.save(0)
s0 = x.state()
prev_hook = int(L[K + 67].split()[2])
acts = []
for d in (-1, 0, 1):
    acts.append((d, 0, None))
    acts.append((d, 1, None))           # hold / launch with the previous aim
    for a in (-20, -12, -6, 0, 6):
        acts.append((d, 1, a))          # (re)launch at angle a (needs release before)
TAIL = 18
def t5671(S):
    if any(s['frz'] for s in S): return None
    for a, b in zip(S, S[1:]):
        if b['x'] >= 5671 > a['x']:
            return a['rt'] + (5671 - a['x']) / (b['x'] - a['x'])
n = P - 1 - K
pre_aim = tuple(int(v) for v in L[K + 67].split()[4:6])
seqs = []; metas = []
for combo in itertools.product(range(len(acts)), repeat=n):
    seq = []; ph = prev_hook; aimv = pre_aim; ok = True
    for ci in combo:
        d, h, a = acts[ci]
        if a is not None:
            if ph:      # relaunch needs a released hook
                ok = False; break
            aimv = aim(a)
        seq.append((d, 0, h, 0, aimv[0], aimv[1])); ph = h
    if not ok: continue
    seqs.append(seq); metas.append(combo)
print('approach sequences', len(seqs), flush=True)
# stage 1: approach only, keep states before P
R = x.runmany(seqs, restore=0)
cands = []
for seq, S in zip(seqs, R):
    if any(s['frz'] for s in S): continue
    e = S[-1]
    if 5280 <= e['x'] <= 5312 and abs(e['y'] - 1948) < 40:
        cands.append((seq, e))
print('candidates', len(cands), flush=True)
best = (None, None)
jobs = []
for seq, e in cands:
    base = math.degrees(math.atan2(min(max(e['y'] - 4, 1922), 1950) - e['y'], 5248 - e['x']))
    for da in range(-12, 13, 2):
        a = aim(base + da)
        for hk in (0, 1):
            fire = (1, 0, hk, 1, a[0], a[1])
            jobs.append(seq + [fire] + [(1, 0, 0, 0, 1000, 0)] * TAIL)
print('jobs', len(jobs), flush=True)
res = []
for c0 in range(0, len(jobs), 2000):
    R = x.runmany(jobs[c0:c0 + 2000], restore=0)
    for j, S in zip(jobs[c0:c0 + 2000], R):
        t = t5671(S)
        if t is not None:
            res.append((t, -S[-1]['vx'], j))
res.sort(key=lambda r: (r[0], r[1]))
for r in res[:5]:
    print('t5671 %.3f vx %.2f' % (r[0], -r[1]))
if res:
    with open(out, 'w') as g:
        g.writelines(L[:K + 68])
        for c in res[0][2]:
            g.write(' '.join(str(v) for v in c) + ' 3\n')
