#!/usr/bin/env python3
"""Teero's shots from the input extraction (teero/teero_inputs_0-3131.csv): fire race tick (true), position on his
track, aim, and the simulated explosion (tick offset, point) of a grenade fired there. Writes teero/shots_sim.tsv.
usage: teero_shots.py"""
import csv, math, os, re, subprocess
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
T = {}
for l in open('teero_track.txt'):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
def tpos(kf):
    k0 = math.floor(kf); f = kf - k0
    a, b = T.get(k0), T.get(k0 + 1)
    if not a or not b: return a or b
    return (a[0] + f * (b[0] - a[0]), a[1] + f * (b[1] - a[1]))
rows = list(csv.DictReader(open('teero/teero_inputs_0-3131.csv')))
out = open('teero/shots_sim.tsv', 'w')
out.write('n\tfire_rt\tx\ty\taim\taim_cert\tdT\tex\tey\n')
n = 0
for i, r in enumerate(rows):
    if r['fire'] != '1': continue
    n += 1
    rt = float(r['s_since_start']) * 50 - 3.2
    # the shot starts at the tee position before the fire step: his track point one tick earlier
    x, y = tpos(rt + 3 - 1)
    cert = float(r['fire_aim_certainty'] or 0)
    aim = r['fire_aim_deg']
    if aim == '' or cert < 0.5:
        # fall back to the per-frame aim around the fire frame with the best certainty
        cands = [rows[j] for j in range(max(0, i - 2), min(len(rows), i + 3)) if rows[j]['aim_angle_deg'] != '']
        best = max(cands, key=lambda q: float(q['aim_certainty'] or 0)) if cands else None
        if best is not None:
            aim, cert = best['aim_angle_deg'], float(best['aim_certainty'] or 0)
    a = math.radians(float(aim))
    tx, ty = round(math.cos(a) * 1000), round(math.sin(a) * 1000)
    cmd = f'quiet;replay runs/ex/e1025.txt;tp {x:.0f} {y:.0f} 0 0;reload 0;loud;in 0 0 0 1 {tx} {ty};nextexp'
    res = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', cmd], capture_output=True, text=True).stdout
    tick_now = re.findall(r't=(\d+)', res)
    m = re.search(r'next explosion at tick (\d+) pos ([\d.-]+) ([\d.-]+)', res)
    if m and tick_now:
        dT = int(m.group(1)) - int(tick_now[-1]); ex, ey = float(m.group(2)), float(m.group(3))
    else:
        dT, ex, ey = 0, x, y  # exploded in the fire step (point-blank)
    out.write(f'{n}\t{rt:.1f}\t{x:.0f}\t{y:.0f}\t{float(aim):.1f}\t{cert:.2f}\t{dT}\t{ex:.0f}\t{ey:.0f}\n')
    print(f'#{n:2d} fire rt {rt:7.1f} at {x:5.0f},{y:5.0f} aim {float(aim):6.1f} (cert {cert:.2f}) -> explodes +{dT:2d} at {ex:5.0f},{ey:5.0f}')
