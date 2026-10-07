#!/usr/bin/env python3
"""Ticks with large v^2 losses (any cause): rt, pos, v before -> after, hook, grounded.  usage: bigloss.py INPUTS [thr=150] [from] [to]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
f = sys.argv[1]; thr = float(sys.argv[2]) if len(sys.argv) > 2 else 150; r0 = int(sys.argv[3]) if len(sys.argv) > 3 else 1030; r1 = int(sys.argv[4]) if len(sys.argv) > 4 else 99999
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + f], capture_output=True, text=True).stdout
R = []
for l in out.splitlines():
    m = re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) jumped (\d+) grounded (\d) reload (\d+) proj (\d+)', l)
    if m:
        g = m.groups()
        R.append(dict(d=int(g[0]), h=int(g[2]), rt=int(g[6]), x=float(g[7]), y=float(g[8]), vx=float(g[9]), vy=float(g[10]), hook=int(g[11]), gr=int(g[13]), pr=int(g[15])))
tot = 0
for p, q in zip(R, R[1:]):
    if q['rt'] < r0 or q['rt'] > r1: continue
    E0 = p['vx'] ** 2 + p['vy'] ** 2 - p['y']; E1 = q['vx'] ** 2 + q['vy'] ** 2 - q['y']
    if E1 - E0 < -thr:
        tot += E1 - E0
        print(f"rt {q['rt']} pos {q['x']:.0f},{q['y']:.0f} v ({p['vx']:.1f},{p['vy']:.1f}) -> ({q['vx']:.1f},{q['vy']:.1f}) dE {E1-E0:+.0f} hook {q['hook']} in_h {q['h']} dir {q['d']} gr {q['gr']}")
print('total', round(tot))
