#!/usr/bin/env python3
"""List grenade shots (fire ticks) and velocity jumps (kicks) of a run.  usage: kicks.py INPUTS [from_rt] [to_rt]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
a = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 0; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 99999
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + a], capture_output=True, text=True).stdout
R = []
for l in out.splitlines():
    m = re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) jumped (\d+) grounded (\d) reload (\d+) proj (\d+)', l)
    if m:
        g = m.groups()
        R.append(dict(d=int(g[0]), j=int(g[1]), h=int(g[2]), f=int(g[3]), tx=int(g[4]), ty=int(g[5]), rt=int(g[6]), x=float(g[7]), y=float(g[8]),
                      vx=float(g[9]), vy=float(g[10]), hook=int(g[11]), gr=int(g[13]), rl=int(g[14]), pr=int(g[15])))
fires = 0
for p, q in zip(R, R[1:]):
    if q['rt'] < r0 or q['rt'] > r1: continue
    if q['f'] and q['rl'] > p['rl']:
        fires += 1
        print(f"FIRE rt {q['rt']:5d} pos {q['x']:6.0f} {q['y']:5.0f} aim {math.degrees(math.atan2(q['ty'], q['tx'])):7.1f}")
    # kick: velocity change beyond gravity/hook/control (> 2.5 px/t)
    dvx, dvy = q['vx'] - p['vx'], q['vy'] - p['vy'] - 0.5
    if math.hypot(dvx, dvy) > 3.0 and q['hook'] <= 0:
        sp, sq = math.hypot(p['vx'], p['vy']), math.hypot(q['vx'], q['vy'])
        print(f"  dv rt {q['rt']:5d} pos {q['x']:6.0f} {q['y']:5.0f} v ({p['vx']:6.1f},{p['vy']:6.1f}) -> ({q['vx']:6.1f},{q['vy']:6.1f}) |v| {sp:5.1f} -> {sq:5.1f}")
print('fires', fires)
