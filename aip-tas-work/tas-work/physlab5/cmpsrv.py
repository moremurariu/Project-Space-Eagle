# cmpsrv.py INPUTS TRACE: compare the real-server trace with xlab states for every input line
import sys, re
from xl import *
f, tr = sys.argv[1], sys.argv[2]
L = open(f).readlines()
T = {}
for l in open(tr):
    m = re.match(r'^(\d+) ([0-9.-]+) ([0-9.-]+) ([0-9.-]+) ([0-9.-]+) frz=(\d)', l)
    if m:
        T[int(m.group(1))] = tuple(float(m.group(k)) for k in range(2, 6)) + (int(m.group(6)),)
x = XL()
out = x.cmd(['loud'] + ['r ' + ' '.join(l.split()[:7]) for l in L] + ['quiet'])
S = [parse_state(l) for l in out if l.startswith('S ')]
bad = 0; maxd = 0
for i, s in enumerate(S, 1):
    t = T.get(i)
    if t is None:
        print('missing trace line', i); bad += 1; continue
    d = max(abs(round(s['x']) - t[0]), abs(round(s['y']) - t[1]))
    dv = max(abs(s['vx'] - t[2]), abs(s['vy'] - t[3]))
    maxd = max(maxd, d)
    if d > 1 or dv > 0.01 or (t[4] != 0) != (s['frz'] != 0):
        if bad < 5:
            print('MISMATCH line', i, 'rt', s['rt'], fmt(s), 'srv', t)
        bad += 1
print(f'{f}: {len(S)} lines, trace {len(T)}, mismatches {bad}, max pos diff {maxd}, final sim {fmt(S[-1])} srv {T.get(len(S))}')
