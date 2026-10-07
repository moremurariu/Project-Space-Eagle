#!/usr/bin/env python3
"""Hill-climbing / LNS over a per-tick input sequence from a start state, evaluated exactly with `lab` (batch mode).
Representation: dirs[i] in {-1,0,1}; hook segments [(start, end, angle_deg)] (hook held for start <= i < end, fired at start).
"""
import random, math, sys, os, json, time
from labx import *

def to_inputs(dirs, segs, n=None):
    n = n or len(dirs)
    H = [None] * n
    for (a, b, ang) in segs:
        for i in range(a, min(b, n)):
            H[i] = ang
    out = []
    last_ang = 90.0
    for i in range(n):
        if H[i] is not None:
            # keep the aim of the segment for every tick of it (the aim after firing does not matter for ground hooks)
            tx, ty = aim(H[i])
            out.append((dirs[i], 0, 1, 0, tx, ty, -1))
        else:
            tx, ty = aim(last_ang)
            out.append((dirs[i], 0, 0, 0, tx, ty, -1))
    return out

def from_inputs(inputs):
    """inputs -> dirs, segs (angle from tx,ty of the firing tick)"""
    dirs = [c[0] for c in inputs]
    segs = []
    i = 0
    while i < len(inputs):
        if inputs[i][2]:
            j = i
            while j < len(inputs) and inputs[j][2]:
                j += 1
            ang = math.degrees(math.atan2(inputs[i][5], inputs[i][4]))
            segs.append((i, j, ang))
            i = j
        else:
            i += 1
    return dirs, segs

def valid(segs, n, first_free=1):
    segs = sorted(segs)
    for k, (a, b, ang) in enumerate(segs):
        if a < first_free or b <= a or a >= n:
            return False
        if k and a <= segs[k - 1][1]:  # need a release tick between hooks
            return False
    return True

def mutate(dirs, segs, n, rng, first_free=1):
    dirs = list(dirs); segs = [list(s) for s in segs]
    for _ in range(rng.choice([1, 1, 1, 2, 3])):
        r = rng.random()
        if r < 0.3:
            i = rng.randrange(n); L = rng.choice([1, 1, 2, 3, 4, 6, 10])
            v = rng.choice([-1, 0, 1])
            for k in range(i, min(n, i + L)):
                dirs[k] = v
        elif r < 0.45 and segs:
            s = rng.choice(segs); s[2] += rng.choice([-1, 1]) * rng.choice([0.2, 0.5, 1, 2, 4, 8])
        elif r < 0.6 and segs:
            s = rng.choice(segs); s[0] += rng.choice([-2, -1, 1, 2])
        elif r < 0.75 and segs:
            s = rng.choice(segs); s[1] += rng.choice([-3, -2, -1, 1, 2, 3])
        elif r < 0.85 and segs:
            segs.remove(rng.choice(segs))
        else:
            a = rng.randrange(first_free, n); L = rng.choice([1, 1, 2, 2, 3, 4, 6, 8, 12, 20])
            segs.append([a, a + L, rng.uniform(-180, 180)])
    segs = [tuple(s) for s in segs if s[1] > s[0]]
    segs.sort()
    return dirs, segs

class Problem:
    def __init__(self, start, n, objective):
        self.start, self.n, self.obj = start, n, objective
        self.evals = 0
    def evaluate(self, cands):
        seqs = [to_inputs(d, s, self.n) for d, s in cands]
        B = batch(self.start, seqs)
        self.evals += len(cands)
        return [self.obj(S) for S in B], B

def hill(prob, dirs, segs, iters=50, pop=300, rng=None, first_free=1, log=True, keep=1):
    rng = rng or random.Random(1)
    (best,), (bS,) = prob.evaluate([(dirs, segs)])
    cur = (dirs, segs)
    if log: print('start J', best)
    for it in range(iters):
        cands = []
        while len(cands) < pop:
            d, s = mutate(cur[0], cur[1], prob.n, rng, first_free)
            if valid(s, prob.n, first_free):
                cands.append((d, s))
        J, B = prob.evaluate(cands)
        k = min(range(len(J)), key=lambda i: J[i])
        if J[k] < best:
            best, cur, bS = J[k], cands[k], B[k]
            if log: print(f'it {it} J {best:.3f} evals {prob.evals}', flush=True)
    return best, cur, bS
