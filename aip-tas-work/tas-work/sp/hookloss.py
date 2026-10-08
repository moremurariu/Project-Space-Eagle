"""Hook pull per tick of a run (pull rebuilt from the hook position after the step's hook update), its |v| change, and
braking episodes (consecutive applied pulls) ranked by speed lost.  usage: hookloss.py RUN [from to]"""
import sys, math
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from trk import trace
run = sys.argv[1]; r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 1000; r1 = int(sys.argv[3]) if len(sys.argv) > 3 else 9999
T, E = trace(run, r0 - 1)
rows = []
for t in sorted(T):
    if t < r0 or t > r1 or t + 1 not in T: continue
    a, b = T[t], T[t + 1]
    if b['hs'] != 5: continue
    p = a['p']; hp = b['hp']
    d = (hp[0] - p[0], hp[1] - p[1]); L = math.hypot(*d)
    if L <= 46: continue
    vx, vy = a['v'][0], a['v'][1] + 0.5
    dr = b['dir']
    if dr: 
        if dr * vx < 5: vx = max(-5, min(5, vx + 1.5 * dr)) if abs(vx) <= 5 or dr * vx < 0 else vx
    else: vx *= 0.95
    hx, hy = d[0] / L * 3, d[1] / L * 3
    if hy > 0: hy *= 0.3
    hx *= 0.95 if (hx < 0 and dr < 0) or (hx > 0 and dr > 0) else 0.75
    s0 = math.hypot(vx, vy); s1 = math.hypot(vx + hx, vy + hy)
    app = s1 < 15 or s1 < s0
    c = (hx * vx + hy * vy) / (math.hypot(hx, hy) * s0 or 1)
    ang = math.degrees(math.acos(max(-1, min(1, c))))
    turn = math.degrees(math.atan2(vx * (vy + hy) - vy * (vx + hx), vx * (vx + hx) + vy * (vy + hy))) if app else 0
    rows.append((t, app, s1 - s0 if app else 0, ang, L, turn, s0))
# episodes
eps = []; cur = None
for r in rows:
    if not r[1]: continue
    if cur and r[0] == cur['t1'] + 1:
        cur['t1'] = r[0]; cur['dv'] += r[2]; cur['turn'] += r[5]; cur['n'] += 1
    else:
        cur = dict(t0=r[0], t1=r[0], dv=r[2], turn=r[5], n=1, v0=r[6]); eps.append(cur)
tot = sum(e['dv'] for e in eps)
print('applied pull ticks %d, |v| change %.1f, |turn| %.0f deg' % (sum(e['n'] for e in eps), tot, sum(abs(e['turn']) for e in eps)))
for e in sorted(eps, key=lambda e: e['dv'])[:int(sys.argv[4]) if len(sys.argv) > 4 else 40]:
    print('rt %4d-%4d n %2d |v| %5.1f d|v| %+6.2f turn %+6.1f deg  (%.2f px/t per 10 deg)' % (e['t0'], e['t1'], e['n'], e['v0'], e['dv'], e['turn'], -e['dv'] / max(1e-3, abs(e['turn'])) * 10))
