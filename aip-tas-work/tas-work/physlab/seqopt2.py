#!/usr/bin/env python3
"""Like seqopt, but candidates also carry jump ticks: cand = (dirs, segs, jumps)."""
import random, math
from labx import *
from seqopt import valid

def to_inputs3(dirs, segs, jumps, n):
    H = [None] * n
    for (a, b, ang) in segs:
        for i in range(max(0, a), min(b, n)):
            H[i] = ang
    J = set(jumps)
    out = []
    for i in range(n):
        if H[i] is not None:
            tx, ty = aim(H[i]); out.append((dirs[i], 1 if i in J else 0, 1, 0, tx, ty, -1))
        else:
            out.append((dirs[i], 1 if i in J else 0, 0, 0, 0, 1000, -1))
    return out

def from_inputs3(inputs):
    dirs = [c[0] for c in inputs]
    segs = []; i = 0
    while i < len(inputs):
        if inputs[i][2]:
            j = i
            while j < len(inputs) and inputs[j][2]: j += 1
            segs.append((i, j, math.degrees(math.atan2(inputs[i][5], inputs[i][4]))))
            i = j
        else:
            i += 1
    jumps = [i for i, c in enumerate(inputs) if c[1] and (i == 0 or not inputs[i - 1][1])]
    return dirs, segs, jumps

def mutate3(dirs, segs, jumps, n, rng, first_free=1, nojump=False):
    dirs = list(dirs); segs = [list(s) for s in segs]; jumps = list(jumps)
    for _ in range(rng.choice([1, 1, 1, 2, 3])):
        r = rng.random()
        if r < 0.27:
            i = rng.randrange(n); L = rng.choice([1, 1, 2, 3, 4, 6, 10])
            v = rng.choice([-1, 0, 1])
            for k in range(i, min(n, i + L)): dirs[k] = v
        elif r < 0.42 and segs:
            s = rng.choice(segs); s[2] += rng.choice([-1, 1]) * rng.choice([0.1, 0.3, 0.6, 1, 2, 4, 8])
        elif r < 0.55 and segs:
            s = rng.choice(segs); s[0] += rng.choice([-2, -1, 1, 2])
        elif r < 0.68 and segs:
            s = rng.choice(segs); s[1] += rng.choice([-3, -2, -1, 1, 2, 3])
        elif r < 0.76 and segs:
            segs.remove(rng.choice(segs))
        elif r < 0.88 or nojump:
            a = rng.randrange(first_free, n); L = rng.choice([1, 1, 2, 2, 3, 4, 6, 8, 12, 20])
            segs.append([a, a + L, rng.uniform(-180, 180)])
        else:
            if jumps and rng.random() < 0.7:
                k = rng.randrange(len(jumps)); jumps[k] = max(0, min(n - 1, jumps[k] + rng.choice([-3, -2, -1, 1, 2, 3])))
            elif jumps and rng.random() < 0.5:
                jumps.pop(rng.randrange(len(jumps)))
            else:
                jumps.append(rng.randrange(n))
    segs = sorted(tuple(s) for s in segs if s[1] > s[0])
    jumps = sorted(set(jumps))
    return dirs, segs, jumps

def hill3(evalf, cand, n, iters=50, pop=300, rng=None, first_free=1, log=True, nojump=False, patience=None):
    rng = rng or random.Random(1)
    (best,), (bS,) = evalf([cand])
    cur = cand; stale = 0
    if log: print('start J', best, flush=True)
    for it in range(iters):
        cands = []
        while len(cands) < pop:
            c = mutate3(*cur, n, rng, first_free, nojump)
            if valid(c[1], n, first_free):
                cands.append(c)
        J, B = evalf(cands)
        k = min(range(len(J)), key=lambda i: J[i])
        if J[k] < best - 1e-9:
            best, cur, bS = J[k], cands[k], B[k]; stale = 0
            if log: print(f'it {it} J {best:.3f}', flush=True)
        else:
            stale += 1
            if patience and stale >= patience: break
    return best, cur, bS
