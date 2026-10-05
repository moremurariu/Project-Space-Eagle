# Phase A without shots: jump tick x hook re-aim, dir -1. Score: first rt with y<=2300 (alive through 30 ticks).
import itertools, time
from xl import *
x = XL(); x.load('pre978w.txt'); x.save(0)
N = 32
seqs, meta = [], []
for j in list(range(0, 9)) + [None]:
    for r in list(range(0, 16)) + [None]:
        for b in (range(185, 271, 5) if r is not None else [0]):
            for dhold in (-1, 0):
                seq = []
                for k in range(N):
                    jump = 1 if k == j else 0
                    if r is None or k < r:
                        h, aimdeg = 1, -95.6   # keep current hook (aim irrelevant while grabbed)
                    elif k == r:
                        h, aimdeg = 0, b
                    else:
                        h, aimdeg = 1, b
                    d = -1 if (r is None or k <= r + 2) else dhold if dhold else -1
                    seq.append((d, jump, h, 0, float(aimdeg)))
                seqs.append(seq); meta.append((j, r, b, dhold))
t = time.time()
R = x.runmany(seqs)
print(len(seqs), 'seqs', time.time() - t, 's')
res = []
for m, S in zip(meta, R):
    if not alive(S):
        continue
    hit = next((i for i, s in enumerate(S) if s['y'] <= 2300), None)
    if hit is None:
        continue
    s = S[hit]
    res.append((S[hit]['rt'] - (2300 - s['y']) / max(1e-3, -s['vy']) * 0, s['vy'], m, s))
res.sort(key=lambda r: (r[0], r[1]))
for r in res[:15]:
    print(r[0], r[2], fmt(r[3]))
