#!/usr/bin/env python3
"""tptest.py K0 K1 [extra seg args...]: start seg from Teero's own state at Teero tick K0 (track position, velocity from the
track over +-2 ticks with the horizontal ramp undone, reload from his shot catalog) and search to gate K1 (Teero tick).
Prints our race tick at the gate vs Teero's (K1-3): how much slower the search is than Teero from the same state."""
import sys, subprocess, re, math, os
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
K0, K1 = int(sys.argv[1]), int(sys.argv[2]); extra = sys.argv[3:]
T = {}
for l in open('teero_track.txt'):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
def ramp(v): return 1.0 if v * 50 < 550 else 1.4 ** (-(50 * v - 550) / 2000)
a, b = T[K0 - 2], T[K0 + 2]; dx, dy = (b[0] - a[0]) / 4, (b[1] - a[1]) / 4
vx = dx
for _ in range(60): vx = dx / ramp(math.hypot(vx, dy))
shots = []
for l in open('teero/catalog/shots.tsv').read().splitlines()[1:]:
    c = l.split('\t'); shots.append(float(c[1]))
last = max([s for s in shots if s <= K0 + 0.5] or [-100])
rel = max(0, 25 - int(round(K0 - last)))
rel = int(os.environ.get('TPREL', rel))
x, y = T[K0]
cmd = ['../ddnet/build-sim/' + os.environ.get('BIN', 'segf'), 'AiP-Gores.map', 'prefix=runs/shaft/p1600.txt', f'tp={x},{y},{vx:.3f},{dy:.3f}', f'tpk={K0-3}',
       f'tpreload={rel}', f'gate={K1}', f'maxticks={2*(K1-K0)+40}', 'out=runs/tp/t_',
       *('horizon=150 beam=6000 threads=' + os.environ.get('TH', '4') + ' quiet=1 survevery=2 survive=30 rothook=1 quant=1 ghost=3 hnow=1000 ghoste=0.02 '
         'sinks=1156,1285,1443,1793,2143,2465 kcredit=2 kready=10 ghostsink=1500 latpen=0.2 latdz=24 jitter=1 seed=1').split(), *extra]
os.makedirs('runs/tp', exist_ok=True)
o = subprocess.run(cmd, capture_output=True, text=True, timeout=int(os.environ.get('TO', 60))).stdout
m = re.search(r'GATE rt (\d+)', o)
r = int(m.group(1)) if m else None
print(f'k{K0}->k{K1} tp ({x:.0f},{y:.0f}) v ({vx:.1f},{dy:.1f}) rel {rel} {" ".join(extra)}: ' + (f'rt {r} vs Teero {K1-3} ({r-(K1-3):+d})' if r else 'NOGATE'))
