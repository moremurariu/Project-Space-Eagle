# Phase A with one shot: fire tick f x aim a, jump tick j, hook re-aim (r,b), dir -1. Score: first rt with y<=2300.
import itertools, time, sys
from xl import *
x = XL(); x.load('pre978w.txt'); x.save(0)
N = 30
seqs, meta = [], []
for f in (0, 1, 2):
    for a in range(-70, 71, 4):
        for j in list(range(0, 12)) + [None]:
            for r in [None, 4, 6, 8, 10]:
                for b in ((200, 220, 235, 250, 265) if r is not None else [0]):
                    seq = []
                    for k in range(N):
                        jump = 1 if k == j else 0
                        fire = 1 if k == f else 0
                        if r is None or k < r:
                            h, hd = 1, -95.6
                        elif k == r:
                            h, hd = 0, b
                        else:
                            h, hd = 1, b
                        aimdeg = a if k == f else hd
                        seq.append((-1, jump, h, fire, float(aimdeg)))
                    seqs.append(seq); meta.append((f, a, j, r, b))
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
    res.append((s['rt'], s['vy'], m, s))
res.sort(key=lambda r: (r[0], r[1]))
for r in res[:25]:
    print(r[0], r[2], fmt(r[3]))
