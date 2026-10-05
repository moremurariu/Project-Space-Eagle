# Climb scan: from the c3 (seed 3) turnaround, release at tick r and launch one long hook at aim a (held), dir d.
# Report: first tick with y <= 2296 (pre-fire height) and first tick with y <= 1955, x there.
import json, math
from xl import *
from hc import seq_tuples
d = json.load(open('runs_b/c3_1038_3.json'))
x = XL(); x.load('pre978w.txt', d['cut']); x.save(0)
base = d['seq']
res = []
seqs, meta = [], []
for r in range(10, 24):          # seq index (rt = 971 + r)
    for a in [x_ / 2 for x_ in range(470, 540)]:   # 235..270 deg
        for dd in (-1, 0):
            s = [list(c) for c in base[:r]]
            s.append([dd, 0, 0, 0, a])
            s += [[dd, 0, 1, 0, a]] * (60 - r)
            for c in s:
                c[3] = 0
            seqs.append(s); meta.append((r, a, dd))
R = x.runmany([seq_tuples(s) for s in seqs])
for m, S in zip(meta, R):
    t1 = next((s for s in S if s['y'] <= 2296), None)
    t2 = next((s for s in S if s['y'] <= 1955), None)
    if t1 is None or t2 is None or any(s['frz'] for s in S if s['rt'] <= t2['rt']):
        continue
    res.append((t2['rt'], t1['rt'], m, t2['x'], t2['vx'], t2['vy']))
res.sort()
for r in res[:15]:
    print(r)
print('baseline c3s3: y<=2296 at 995, y<=1955 at 1023/1024')
