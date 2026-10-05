#!/usr/bin/env python3
"""Energy accounting of a run: per tick dE = (v^2 - y) change, attributed to a cause.  usage: eacct.py INPUTS [from_rt] [to_rt]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
f = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 1030; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 99999
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + f], capture_output=True, text=True).stdout
R = []
for l in out.splitlines():
    m = re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) jumped (\d+) grounded (\d) reload (\d+) proj (\d+)', l)
    if m:
        g = m.groups()
        R.append(dict(d=int(g[0]), j=int(g[1]), h=int(g[2]), f=int(g[3]), rt=int(g[6]), x=float(g[7]), y=float(g[8]), vx=float(g[9]), vy=float(g[10]),
                      hook=int(g[11]), gr=int(g[13]), rl=int(g[14]), pr=int(g[15])))
cat = {}
def add(k, v):
    a = cat.setdefault(k, [0.0, 0, 0.0])
    if v > 0: a[0] += v
    else: a[2] += v
    a[1] += 1
for p, q in zip(R, R[1:]):
    if q['rt'] < r0 or q['rt'] > r1: continue
    E0 = p['vx'] ** 2 + p['vy'] ** 2 - p['y']; E1 = q['vx'] ** 2 + q['vy'] ** 2 - q['y']
    dE = E1 - E0
    # cause
    expl = q['pr'] < p['pr'] or (q['f'] and q['rl'] > p['rl'] and abs(dE) > 30)
    if expl and abs(dE) > 30: c = 'kick'
    elif q['j'] and not p['j'] and q['vy'] < -10: c = 'jump'
    elif q['hook'] > 0 or (q['h'] and q['hook'] >= 0 and q['hook'] != 0): c = 'hook'
    elif q['gr']: c = 'ground'
    elif abs(q['vx']) < 1e-6 and abs(p['vx']) > 2 or abs(q['vy']) < 1e-6 and abs(p['vy']) > 2: c = 'wall'
    elif q['d'] == 0 and abs(p['vx']) > 1: c = 'dir0'
    elif q['d'] != 0 and q['d'] * p['vx'] < 0 and abs(p['vx']) > 5: c = 'brake'
    else: c = 'free'
    add(c, dE)
tot = 0
for k, (pos, n, neg) in sorted(cat.items(), key=lambda kv: kv[1][2]):
    print(f'{k:7s} ticks {n:5d}  gain {pos:+9.0f}  loss {neg:+9.0f}  net {pos+neg:+9.0f}')
    tot += pos + neg
print('total', round(tot))
