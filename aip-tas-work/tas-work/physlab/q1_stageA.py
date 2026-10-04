import sys, pickle
from q1lib import *
seed = int(sys.argv[1]); iters = int(sys.argv[2]); src = sys.argv[3]; lamv = float(sys.argv[4]) if len(sys.argv) > 4 else 0.3
N = 50
s296 = state_after('../kog_pregren_992.txt', 364)
def obj(S):
    t, i = cross(S, 450, 9312, 9472)
    if t is None:
        return 1000
    # survive 6 more ticks inside the shaft and credit downward speed
    for s in S[i:i + 6]:
        if s['frz']: return 900
    return t - lamv * S[i]['vy']
P = Problem(s296, N, obj)
if src.endswith('.pkl'):
    L = pickle.load(open(src, 'rb'))
    d, s = L[seed % len(L)][1]
elif src == 'base':
    base = read_inputs('../kog_pregren_992.txt')[364:364 + N]
    base[0] = (base[0][0], 0, 0, 0, base[0][4], base[0][5], -1)
    d, s = from_inputs(base)
else:
    base = read_inputs(src)
    off = int(sys.argv[5]) if len(sys.argv) > 5 else 0
    base = base[off:off + N]
    base[0] = (base[0][0], 0, 0, 0, base[0][4], base[0][5], -1)
    d, s = from_inputs(base)
J, cur, S = hill(P, d, s, iters=iters, pop=300, rng=random.Random(seed), log=False)
t, i = cross(S, 450, 9312, 9472)
print(f'seed {seed} src {src} J {J:.3f} t450 {t} {fmt(S[i]) if t else ""}')
print('segs', [(a, b, round(c, 2)) for a, b, c in cur[1]], 'dirs', ''.join('+0-'[1 - x] for x in cur[0]))
with open(f'runs_q1/A_{os.path.basename(src)}_{seed}.txt', 'w') as f:
    for c in to_inputs(cur[0], cur[1], N):
        f.write(' '.join(map(str, c)) + '\n')
