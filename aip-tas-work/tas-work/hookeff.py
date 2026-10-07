#!/usr/bin/env python3
"""Hook steering efficiency: per hooked tick (grabbed, no kick), velocity rotation and v^2 change.
usage: hookeff.py INPUTS [from_rt] [to_rt]"""
import re, subprocess, sys, os, math
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
f = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 1030; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 99999
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + f], capture_output=True, text=True).stdout
R = []
for l in out.splitlines():
    m = re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) jumped (\d+) grounded (\d) reload (\d+) proj (\d+)', l)
    if m:
        g = m.groups()
        R.append(dict(h=int(g[2]), rt=int(g[6]), x=float(g[7]), y=float(g[8]), vx=float(g[9]), vy=float(g[10]), hook=int(g[11]), gr=int(g[13]), pr=int(g[15])))
bins = {}
tot_rot = 0; tot_loss = 0; n = 0; nacc = 0; grav_rot = 0
for p, q in zip(R, R[1:]):
    if q['rt'] < r0 or q['rt'] > r1 or q['hook'] <= 0 or q['pr'] < p['pr'] or q['gr']: continue
    v0 = (p['vx'], p['vy'] + 0.5)  # after gravity
    v1 = (q['vx'], q['vy'])
    s0 = math.hypot(*v0); s1 = math.hypot(*v1)
    if s0 < 15: continue
    a = math.atan2(v1[1], v1[0]) - math.atan2(v0[1], v0[0])
    a = (a + math.pi) % (2 * math.pi) - math.pi
    d = s1 * s1 - s0 * s0
    n += 1
    if abs(d) < 1e-3 and abs(a) < 1e-4: continue
    nacc += 1
    tot_rot += abs(a); tot_loss += d
    b = int(min(-d, 299) // 25) * 25 if d < 0 else -1
    bins.setdefault(b, [0, 0.0, 0.0]); bins[b][0] += 1; bins[b][1] += abs(a); bins[b][2] += d
print(f'hooked ticks at |v|>=15: {n}, with a pull applied: {nacc}; total rotation {math.degrees(tot_rot):.0f} deg, v^2 change {tot_loss:.0f}')
print(f'average loss per degree: {-tot_loss / max(math.degrees(tot_rot), 1e-6):.1f}')
for b in sorted(bins):
    c, r, d = bins[b]
    print(f'loss bin {b:4d}..{b+25:4d}: {c:4d} ticks, rotation {math.degrees(r):6.0f} deg, v^2 {d:8.0f}, per deg {(-d / max(math.degrees(r), 1e-6)):6.1f}')
