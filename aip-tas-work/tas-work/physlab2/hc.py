#!/usr/bin/env python3
"""(1+lambda) hill climbing over per-tick input sequences, evaluated with xlab (physlab2/xl.py).
A sequence is a list of [dir, jump, hook, fire, aim_deg] (aim used for hook launches and shots; y down, 0 = right).
"""
import math
import random

from xl import alive


def seq_tuples(seq):
    return [(int(c[0]), int(c[1]), int(c[2]), int(c[3]), float(c[4])) for c in seq]


def mutate(seq, rng, aims=None, nfire_max=3, allow_fire=True):
    s = [list(c) for c in seq]
    n = len(s)
    k = rng.random()
    i = rng.randrange(n)
    if k < 0.18:  # dir segment
        j = min(n, i + rng.randint(1, 8))
        d = rng.choice((-1, 0, 1))
        for t in range(i, j):
            s[t][0] = d
    elif k < 0.28:  # jump toggle / move
        js = [t for t in range(n) if s[t][1]]
        if js and rng.random() < 0.6:
            t = rng.choice(js)
            s[t][1] = 0
            if rng.random() < 0.7:
                s[max(0, min(n - 1, t + rng.randint(-3, 3)))][1] = 1
        else:
            s[i][1] = 1 - s[i][1]
    elif k < 0.52:  # hook segment with a launch aim
        j = min(n, i + rng.randint(1, 12))
        a = rng.choice(aims) if (aims and rng.random() < 0.5) else rng.uniform(0, 360)
        if i > 0:
            s[i - 1][2] = 0
        for t in range(i, j):
            s[t][2] = 1
            if not s[t][3]:
                s[t][4] = a
        if rng.random() < 0.5 and j < n:
            s[j][2] = 0
    elif k < 0.60:  # release segment
        j = min(n, i + rng.randint(1, 4))
        for t in range(i, j):
            s[t][2] = 0
    elif k < 0.75:  # aim perturbation at a launch / shot tick
        L = [t for t in range(n) if s[t][3] or (s[t][2] and (t == 0 or not s[t - 1][2]))]
        if L:
            t = rng.choice(L)
            da = rng.choice((0.05, 0.2, 1, 3, 10))
            a = s[t][4] + rng.uniform(-da, da)
            # a hook launch keeps its aim for the whole held segment (aim only matters at launch)
            u = t
            while u < n and (u == t or (s[u][2] and not s[u][3])):
                s[u][4] = a
                u += 1
    elif allow_fire:  # shots
        fs = [t for t in range(n) if s[t][3]]
        r = rng.random()
        if fs and r < 0.4:
            t = rng.choice(fs)
            s[t][3] = 0
            t2 = max(0, min(n - 1, t + rng.randint(-3, 3)))
            s[t2][3] = 1
            s[t2][4] = s[t][4] + rng.uniform(-5, 5)
        elif fs and r < 0.6:
            s[rng.choice(fs)][3] = 0
        elif len(fs) < nfire_max:
            s[i][3] = 1
            s[i][4] = rng.uniform(0, 360)
    return s


def climb(x, slot, seq, score, iters=200, lam=100, seed=1, aims=None, log=None, allow_fire=True, nfire_max=3):
    """score(states) -> tuple (lower is better) or None if invalid. Returns best (score, seq)."""
    rng = random.Random(seed)
    R = x.runmany([seq_tuples(seq)], restore=slot)
    best = score(R[0])
    bestseq = seq
    for it in range(iters):
        cands = []
        for _ in range(lam):
            m = bestseq
            for _ in range(rng.choice((1, 1, 1, 2, 3))):
                m = mutate(m, rng, aims, nfire_max, allow_fire)
            cands.append(m)
        R = x.runmany([seq_tuples(c) for c in cands], restore=slot)
        for c, S in zip(cands, R):
            sc = score(S)
            if sc is not None and (best is None or sc <= best):
                best, bestseq = sc, c
        if log and it % 20 == 0:
            log(it, best)
    return best, bestseq


def gate_score(ygate=None, xmin=None, xmax=None, cond=None, need_gren=True, hold=0, tie=None):
    """first tick satisfying the gate (alive up to there + hold ticks); tie-break by tie(state) (lower better)"""
    def f(S):
        for i, s in enumerate(S):
            if s['frz']:
                return None
            ok = True
            if ygate is not None and s['y'] > ygate:
                ok = False
            if xmin is not None and s['x'] < xmin:
                ok = False
            if xmax is not None and s['x'] > xmax:
                ok = False
            if cond is not None and not cond(s):
                ok = False
            if need_gren and not s['gren']:
                ok = False
            if ok:
                if any(t['frz'] for t in S[i:i + hold + 1]):
                    return None
                return (s['rt'], tie(s) if tie else 0)
        return None
    return f
