# seedscan.py PREFIX S1JSON OLDFILE CUT: for each s1 landing variant (grounded G), append OLDFILE inputs from rt OLD_G+2 on
# (OLD_G given), evaluate the fractional y=2290 gate; print the best
import sys, json
from xl import *
from hc5 import *
from hc import gate_score
pre, s1f, oldf, cut, oldg = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
d = json.load(open(s1f))
old = from_lines(open(oldf).readlines())
x = XL(); x.load(pre, cut + 68); x.save(0)
g = gate_score(ygate=2290, xmax=5376, hold=6, cond=lambda s: s['gren'] == 2 and s['reload'] == 0 and s['vy'] <= -12, tie=lambda s: s['y'])
res = []
for k, e in enumerate(d):
    G = e['rt']
    for dsh in (-1, 0, 1):
        tail = old[oldg + 1 + dsh + 68:]      # lines producing old rt oldg+2+dsh ...
        seq = [norm(c) for c in e['seq']] + tail
        seq = seq[:1000 - cut]
        S = x.runmany([tuples(seq)], restore=0)[0]
        r = g(S)
        if r is not None:
            i = next(j for j, s in enumerate(S) if s['rt'] == r[0]); p = S[i - 1]
            res.append((p['rt'] + (p['y'] - 2290) / (p['y'] - S[i]['y']), k, G, dsh))
res.sort()
print(len(res), res[:10])
