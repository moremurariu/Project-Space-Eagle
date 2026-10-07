# Pre-fire variants: from a json (cut + seq) take the seq up to step M (exclusive), fire the pre-fire on step M with the
# gmodel aim for each n in NLIST (several aims per n), check the explosion with xlab, and write prefix files ending right
# after the pre-fire step (for xrv).  usage: pf6.py PREFIX JSON M_RT NLIST OUTDIR [K aims per n]
import sys, json, math
from xl import *
from hc5 import *
from gmodel import face_hits
pre, jf, mrt, nl, od = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4], sys.argv[5]
K = int(sys.argv[6]) if len(sys.argv) > 6 else 3
d = json.load(open(jf)); cut = d['cut']
seq = [norm(c) for c in (d.get('base') or d['seq'])]
for c in seq:
    c[3] = 0
m = mrt - cut - 1          # seq index of the step producing rt mrt
x = XL(); x.load(pre, cut + 68); x.save(0)
S = x.runmany([tuples(seq[:m])], restore=0)[0]
p = S[-1]
print('fire from', fmt(p))
L = open(pre).readlines()[:cut + 68]
for n in [int(v) for v in nl.split(',')]:
    hits = face_hits(p['x'], p['y'], n, samples=60)
    if not hits:
        print('n', n, 'no hits'); continue
    idx = sorted(set([len(hits) // 2] + [int(len(hits) * (k + 0.5) / K) for k in range(K)]))
    for k in idx:
        a, yc = hits[k]
        c = list(seq[m]); c[3] = 1; setaim([c], 0, a) if False else None
        c[4] = a; c[5] = None
        if c[2] and (m == 0 or not seq[m - 1][2]):
            c[2] = 0
        s2 = seq[:m] + [c]
        x.restore(0)
        x.run(tuples(s2), loud=False)
        ex = x.explosions()
        out = f'{od}/pf_{mrt}_n{n}_{k}.txt'
        with open(out, 'w') as f:
            f.writelines(L)
            for cc in s2:
                f.write(line(cc) + '\n')
        print('n', n, 'aim %.3f yc %.1f' % (a, yc), 'exp', ex, '->', out)
