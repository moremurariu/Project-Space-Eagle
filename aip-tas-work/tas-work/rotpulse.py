#!/usr/bin/env python3
"""Hand test of 'rotation pulses': from a prefix, every other tick fire a hook at the angle that maximises vx
while keeping |v| (lossless rotation), if a solid tile is within the first-tick hook reach (42..122 px).
usage: rotpulse.py PREFIX NTICKS [mode=up|down|any]"""
import math, re, subprocess, sys, os
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
M = open('map.txt').read().splitlines()
def solid(x, y):
    tx, ty = int(x // 32), int(y // 32)
    if ty < 0 or ty >= len(M) or tx < 0 or tx >= len(M[ty]): return True
    return M[ty][tx] == '#'
def state(lines):
    open('/tmp/_rp.txt' if False else SCR, 'w').write('\n'.join(lines) + '\n')
    out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + SCR], capture_output=True, text=True).stdout
    l = [l for l in out.splitlines() if l.startswith('in ')][-1]
    m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)', l)
    return int(m.group(1)), float(m.group(3)), float(m.group(4)), float(m.group(5)), float(m.group(6)), l
SCR = os.path.join(HERE, 'runs/c1x/_rp.txt')
pre = open(sys.argv[1]).read().splitlines()
N = int(sys.argv[2]); mode = sys.argv[3] if len(sys.argv) > 3 else 'any'
lines = list(pre)
prev_hook = 0
for k in range(N):
    rt, x, y, vx, vy, l = state(lines)
    print(f'rt {rt} x {x:.0f} y {y:.0f} v {vx:.3f} {vy:.3f} |v| {math.hypot(vx,vy):.3f} prog-quantum {vx*1.4**(-(50*math.hypot(vx,vy)-550)/2000):.2f}')
    best = None
    if not prev_hook:
        vpx, vpy = vx, vy + 0.5
        L0 = math.hypot(vpx, vpy)
        for i in range(-720, 720):
            th = math.radians(i / 4)
            c, s = math.cos(th), math.sin(th)
            if mode == 'down' and s <= 0: continue
            if mode == 'up' and s >= 0: continue
            hx, hy = 3 * c, 3 * s
            if hy > 0: hy *= 0.3
            hx *= 0.95 if hx > 0 else 0.75
            nx, ny = vpx + hx, vpy + hy
            Ln = math.hypot(nx, ny)
            if not (Ln < 15 or Ln < L0): continue
            if nx <= vpx + 0.02: continue
            hit = None
            r = 42
            while r <= 122:
                if solid(x + c * r, y + s * r): hit = r; break
                r += 1
            if hit is None or hit <= 46: continue
            if best is None or nx > best[0]: best = (nx, ny, c, s, hit)
    if best:
        tx, ty = round(best[2] * 1000), round(best[3] * 1000)
        lines.append(f'1 0 1 0 {tx} {ty} -1'); prev_hook = 1
        print(f'   pulse aim {tx} {ty} anchor r {best[4]} -> v {best[0]:.3f} {best[1]:.3f}')
    else:
        lines.append('1 0 0 0 0 1 -1'); prev_hook = 0
open(os.path.join(HERE, 'runs/c1x/rp_out.txt'), 'w').write('\n'.join(lines) + '\n')
