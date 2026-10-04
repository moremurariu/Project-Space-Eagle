from labx import *
import math
# One-tick rotation pulses: tee ~85-90 px from a hookable surface, hook fired and released after 1 tick.
# falling (vy>0): ceiling above at (8500,700) [solid underside y 608, freeze row 19 is hook-transparent]
# rising (vy<0): spawn-room floor below at (400,300) [floor y 384]
def scan(vx, vy, d=1):
    x, y = (8500, 700) if vy > 0 else (400, 300)
    st = dict(x=x, y=y, vx=vx, vy=vy, jumped=0)
    angs = [a / 4 for a in range(-179 * 4, 180 * 4)]
    base = batch(st, [[(d, 0, 0, 0, 0, 1000, -1)]])[0][0]
    B = batch(st, [[(d, 0, 1, 0) + aim(a) + (-1,)] for a in angs], chunk=2000)
    out = []
    th0 = math.degrees(math.atan2(base['vy'], base['vx']))
    for a, S in zip(angs, B):
        s = S[0]
        if s['vx'] == base['vx'] and s['vy'] == base['vy']: continue
        th = math.degrees(math.atan2(s['vy'], s['vx']))
        out.append((a, th0 - th, s['v'] - base['v'], s['vx'], s['vy']))
    return base, out
if __name__ == '__main__':
    print('one-tick rotation pulse, dir 1: best turn of v toward horizontal (+x) with |v| not increased')
    for vx, vy in [(20, 15), (20, -15), (25, 8), (25, -8), (15, 20), (15, -20), (30, 5), (30, -5)]:
        base, out = scan(vx, vy)
        sgn = 1 if vy > 0 else -1
        c = [r for r in out if r[2] <= 0.001]
        b = max(c, key=lambda r: sgn * r[1])
        print(f"v=({vx},{vy:+}) (after gravity {base['vx']:.2f},{base['vy']:.2f}): aim {b[0]:7.2f} deg -> rotation {abs(b[1]):.2f} deg/tick, d|v| {b[2]:+.3f}, v=({b[3]:.2f},{b[4]:.2f}) dvx {b[3]-base['vx']:+.2f}")
