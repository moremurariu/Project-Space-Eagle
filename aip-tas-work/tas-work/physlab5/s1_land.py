# Stage 1: from a cut of the new prefix, find inputs that ground on the block top (refill the air jump) while
# keeping the downward speed (slide past the block corner).  usage: s1_land.py PREFIX CUT_RT N ITERS SEED
import sys, json, random, time
from xl import *
from hc5 import *
pre, cut, N, iters, seed = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
x = XL(); x.load(pre, cut + 68); x.save(0)
L = open(pre).readlines()[cut + 68:]
init = from_lines(L)[:N]
while len(init) < N:
    init.append([1, 0, 0, 0, 0.0, None])
rng = random.Random(seed)
found = {}
def score(S):
    for i in range(len(S) - 1):
        s = S[i]
        if s['frz']:
            return None
        if s['gr'] and not (S[i + 1]['jumped'] & 2) and S[i + 1]['vy'] > 10 and not S[i + 1]['frz']:
            return (s['rt'], -S[i + 1]['vy'], i)
    return None
t0 = time.time()
pool = [init]
for it in range(iters):
    cands = []
    for _ in range(200):
        m = rng.choice(pool)
        for _ in range(rng.choice((1, 1, 2, 3, 4))):
            m = mutate(m, rng, None, 0, False, nojump=True)
        cands.append(m)
    R = x.runmany([tuples(c) for c in cands], restore=0)
    for c, S in zip(cands, R):
        sc = score(S)
        if sc is None:
            continue
        i = sc[2]
        s1 = S[i + 1]
        key = (sc[0], round(s1['x']), round(s1['y']))
        if key not in found or found[key][0] > sc[1]:
            found[key] = (sc[1], c[:i + 2], (s1['x'], s1['y'], s1['vx'], s1['vy']))
            pool.append(c)
            if len(pool) > 80:
                pool.pop(1)
    if it % 20 == 0:
        best = sorted(found.items(), key=lambda kv: (kv[0][0], kv[1][0]))[:3]
        print(it, len(found), [(k, v[0], tuple(round(z, 2) for z in v[2])) for k, v in best], f'{time.time()-t0:.0f}s', flush=True)
res = sorted(found.items(), key=lambda kv: (kv[0][0], kv[1][0]))
json.dump([dict(rt=k[0], st=v[2], vy=-v[0], seq=v[1]) for k, v in res[:200]], open(f'runs/s1_{cut}_{seed}.json', 'w'))
for k, v in res[:15]:
    print(k, v[0], tuple(round(z, 2) for z in v[2]))
