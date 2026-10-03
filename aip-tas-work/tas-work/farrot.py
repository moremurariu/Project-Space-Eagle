#!/usr/bin/env python3
"""Far rotation pulse test: from PREFIX, fire a hook at angle a (fine steps), hold it until the first tick its pull
is applied, then release; report vx / |v| after. usage: farrot.py PREFIX [amin amax step] [hold=6]"""
import math, re, subprocess, sys, os
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
LAB = '../ddnet/build-sim/lab'
pre = open(sys.argv[1]).read().splitlines()
a0, a1, st = (float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4])) if len(sys.argv) > 4 else (-100, 0, 0.5)
HOLD = int(sys.argv[5]) if len(sys.argv) > 5 else 6
TMP = 'runs/c1x/_fr.txt'
def run(lines):
    open(TMP, 'w').write('\n'.join(lines) + '\n')
    out = subprocess.run([LAB, 'AiP-Gores.map', 'replay ' + TMP], capture_output=True, text=True).stdout
    S = []
    for l in out.splitlines():
        m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+)', l)
        if m and l.startswith('in '): S.append((int(m.group(1)), float(m.group(3)), float(m.group(4)), float(m.group(5)), float(m.group(6)), int(m.group(7))))
    return S
base = run(pre + ['1 0 0 0 0 1 -1'] * HOLD)
b = base[len(pre) - 1]
print(f'start rt {b[0]} x {b[1]:.0f} y {b[2]:.0f} v {b[3]:.3f} {b[4]:.3f} |v| {math.hypot(b[3], b[4]):.3f}')
res = []
a = a0
while a <= a1:
    tx, ty = round(math.cos(math.radians(a)) * 1000), round(math.sin(math.radians(a)) * 1000)
    lines = pre + [f'1 0 1 0 {tx} {ty} -1'] * HOLD
    S = run(lines)
    # first tick whose velocity differs from free flight
    k0 = None
    for k in range(HOLD):
        s, f = S[len(pre) + k], base[len(pre) + k]
        if abs(s[3] - f[3]) > 1e-3 or abs(s[4] - f[4]) > 1e-3:
            k0 = k; break
    if k0 is not None:
        lines = pre + [f'1 0 1 0 {tx} {ty} -1'] * (k0 + 1) + ['1 0 0 0 0 1 -1'] * (HOLD - k0 - 1)
        S = run(lines)
        s = S[len(pre) + HOLD - 1]; f = base[len(pre) + HOLD - 1]
        res.append((s[3] - f[3], a, k0, s[3], s[4], math.hypot(s[3], s[4]) - math.hypot(f[3], f[4]), s[1] - f[1], s[2] - f[2]))
    a += st
res.sort(reverse=True)
for r in res[:12]:
    print(f'aim {r[1]:6.1f} pull at +{r[2]}  dvx {r[0]:+.3f} -> v {r[3]:.3f} {r[4]:.3f}  d|v| {r[5]:+.3f}  dx {r[6]:+.0f} dy {r[7]:+.0f}')
