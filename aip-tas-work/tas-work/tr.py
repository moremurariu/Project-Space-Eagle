#!/usr/bin/env python3
"""tr.py INPUTS [rt0] [rt1]: per-tick table (rt, input, pos, vel, |v|, E = v^2 - y, hook state, jumped, grounded, grenade)"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__))
f = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else -999; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 99999
out = subprocess.run([os.path.join(HERE, '../ddnet/build-sim/lab'), os.path.join(HERE, 'AiP-Gores.map'), 'replay ' + os.path.abspath(f)], capture_output=True, text=True).stdout
for l in out.splitlines():
    m = re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) jumped (\d+) grounded (\d) .*frz (\d) gren (\d)', l)
    if not m: continue
    g = m.groups()
    rt = int(g[6])
    if rt < r0 or rt > r1: continue
    x, y, vx, vy = map(float, g[8:12])
    ang = math.degrees(math.atan2(int(g[5]), int(g[4])))
    print(f'rt {rt:4d} t {g[7]:>4s} in {int(g[0]):2d} {g[1]} {g[2]} {g[3]} aim {ang:7.1f} | pos {x:7.1f} {y:7.1f} vel {vx:7.2f} {vy:7.2f} |v| {math.hypot(vx, vy):6.2f} E {vx*vx+vy*vy-y:8.1f} hs {int(g[12]):2d} j {g[13]} gr {g[14]} frz {g[15]} gren {g[16]}')
