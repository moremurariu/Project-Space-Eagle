"""physlab5: hill climbing with exact-aim support. A sequence element is [dir, jump, hook, fire, deg, ov]
where ov is None or [tx, ty] (exact aim from an input file; cleared whenever the aim is mutated)."""
import math, random
from xl import aim

def tup(c):
    if len(c) > 5 and c[5] is not None:
        return (int(c[0]), int(c[1]), int(c[2]), int(c[3]), int(c[5][0]), int(c[5][1]))
    tx, ty = aim(float(c[4]))
    return (int(c[0]), int(c[1]), int(c[2]), int(c[3]), tx, ty)

def tuples(seq):
    return [tup(c) for c in seq]

def from_lines(lines):
    out = []
    for l in lines:
        p = [int(v) for v in l.split()[:6]]
        out.append([p[0], p[1], p[2], p[3], math.degrees(math.atan2(p[5], p[4])), [p[4], p[5]]])
    return out

def norm(c):
    c = list(c)
    while len(c) < 6:
        c.append(None)
    return c

def setaim(s, t, a):
    s[t][4] = a
    s[t][5] = None

def mutate(seq, rng, aims=None, nfire_max=3, allow_fire=True, lo=0, nojump=False):
    s = [norm(c) for c in seq]
    n = len(s)
    k = rng.random()
    i = rng.randrange(lo, n)
    if k < 0.18:
        j = min(n, i + rng.randint(1, 8))
        d = rng.choice((-1, 0, 1))
        for t in range(i, j):
            s[t][0] = d
    elif k < 0.28:
        if nojump:
            return s
        js = [t for t in range(lo, n) if s[t][1]]
        if js and rng.random() < 0.6:
            t = rng.choice(js)
            s[t][1] = 0
            if rng.random() < 0.7:
                s[max(lo, min(n - 1, t + rng.randint(-3, 3)))][1] = 1
        else:
            s[i][1] = 1 - s[i][1]
    elif k < 0.52:
        j = min(n, i + rng.randint(1, 12))
        a = rng.choice(aims) if (aims and rng.random() < 0.5) else rng.uniform(0, 360)
        if i > lo:
            s[i - 1][2] = 0
        for t in range(i, j):
            s[t][2] = 1
            if not s[t][3]:
                setaim(s, t, a)
        if rng.random() < 0.5 and j < n:
            s[j][2] = 0
    elif k < 0.60:
        j = min(n, i + rng.randint(1, 4))
        for t in range(i, j):
            s[t][2] = 0
    elif k < 0.75:
        L = [t for t in range(lo, n) if s[t][3] or (s[t][2] and (t == 0 or not s[t - 1][2]))]
        if L:
            t = rng.choice(L)
            da = rng.choice((0.05, 0.2, 1, 3, 10))
            a = s[t][4] + rng.uniform(-da, da)
            u = t
            while u < n and (u == t or (s[u][2] and not s[u][3])):
                setaim(s, u, a)
                u += 1
    elif allow_fire:
        fs = [t for t in range(lo, n) if s[t][3]]
        r = rng.random()
        if fs and r < 0.4:
            t = rng.choice(fs)
            s[t][3] = 0
            t2 = max(lo, min(n - 1, t + rng.randint(-3, 3)))
            s[t2][3] = 1
            setaim(s, t2, s[t][4] + rng.uniform(-5, 5))
        elif fs and r < 0.6:
            s[rng.choice(fs)][3] = 0
        elif len(fs) < nfire_max:
            s[i][3] = 1
            setaim(s, i, rng.uniform(0, 360))
    return s

def line(c, w=3):
    t = tup(c)
    return f'{t[0]} {t[1]} {t[2]} {t[3]} {t[4]} {t[5]} {w}'
