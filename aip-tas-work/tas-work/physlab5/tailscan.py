# tailscan.py FILE KEEP_RT: after rt KEEP_RT, try one hook (launch tick, aim, hold) + dir 1; report the best t5671
import sys, math
from xl import *
f, keep = sys.argv[1], int(sys.argv[2])
x = XL(); x.load(f, keep + 68); x.save(0)
N = 20
def t5671(S):
    if any(s['frz'] for s in S): return None
    for a, b in zip(S, S[1:]):
        if b['x'] >= 5671 > a['x']:
            return a['rt'] + (5671 - a['x']) / (b['x'] - a['x'])
seqs = [[(1, 0, 0, 0, 1000, 0)] * N]
meta = [(0, 0, 0)]
for t0 in range(0, 8):
    for deg in range(-90, 91, 3):
        for hold in range(1, 9):
            s = [(1, 0, 0, 0, 1000, 0)] * N
            tx, ty = aim(deg)
            for k in range(t0, min(N, t0 + hold)):
                s[k] = (1, 0, 1, 0, tx, ty)
            seqs.append(s); meta.append((keep + 1 + t0, deg, hold))
R = x.runmany(seqs, restore=0)
res = sorted((t5671(S), m) for S, m in zip(R, meta) if t5671(S) is not None)
print('plain', t5671(R[0]))
print(res[:8])
