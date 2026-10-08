"""Teero's implied |v| along his track: smoothed displacement (dx, dy) per tick, vx solved from dx = vx * ramp(|v|)
(horizontal-only velocity ramp, vy unramped). Compared with our |v| at the same place.  usage: teerov.py RUN [half] [step]"""
import sys, math
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from trk import *
run = sys.argv[1]; H = int(sys.argv[2]) if len(sys.argv) > 2 else 3; step = int(sys.argv[3]) if len(sys.argv) > 3 else 10
R = track()
def ramp(s):
    v = s * 50
    return 1.0 if v < 550 else 1.4 ** (-(v - 550) / 2000)
def solve_vx(dx, vy):
    lo, hi = 0.0, 300.0
    if dx < 0: return -solve_vx(-dx, vy)
    # dx = vx*ramp(hypot(vx,vy)) increasing in vx up to ~119; cap
    for _ in range(60):
        m = (lo + hi) / 2
        if m * ramp(math.hypot(m, vy)) < dx: lo = m
        else: hi = m
        if hi > 250: break
    return (lo + hi) / 2
def tv(k):
    if k - H not in R or k + H not in R: return None
    a, b = R[k - H], R[k + H]
    dx, dy = (b[0] - a[0]) / (2 * H), (b[1] - a[1]) / (2 * H)
    if abs(dx) > 47.9: return (dx, dy, float('inf'))
    vx = solve_vx(dx, dy)
    return dx, dy, math.hypot(vx, dy)
T, E = trace(run, 960)
L = lagmap(T, R, 977)
for t in range(1000, max(T), step):
    if t not in L: continue
    k = L[t][1]
    r = tv(int(round(k)))
    if not r: continue
    # ours smoothed the same way
    if t - H in T and t + H in T:
        a, b = T[t - H]['p'], T[t + H]['p']
        odx, ody = (b[0] - a[0]) / (2 * H), (b[1] - a[1]) / (2 * H)
    else: odx = ody = 0
    print('%4d k %7.1f | ours d %5.1f,%5.1f |v| %5.1f | teero d %5.1f,%5.1f implied |v| %6.1f' % (t, k, odx, ody, T[t]['s'], r[0], r[1], r[2]))
