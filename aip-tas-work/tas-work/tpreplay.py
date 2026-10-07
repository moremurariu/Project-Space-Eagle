#!/usr/bin/env python3
"""Replay a seg output made with tp=: prefix lines, teleport, reload, rest; print lag vs Teero (track frame).
usage: tpreplay.py INPUTS PREFIXLINES x,y,vx,vy TPK RELOAD [every]"""
import re, subprocess, sys, os, math, tempfile
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
f, n, tp, tpk, rl = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
ev = int(sys.argv[6]) if len(sys.argv) > 6 else 5
L = open(f).read().splitlines()
d = tempfile.mkdtemp()
open(f'{d}/a.txt', 'w').write('\n'.join(L[:n]) + '\n'); open(f'{d}/b.txt', 'w').write('\n'.join(L[n:]) + '\n')
x, y, vx, vy = tp.split(',')
out = subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', f'quiet;replay {d}/a.txt;tp {x} {y} {vx} {vy};reload {rl};loud;replay {d}/b.txt'], capture_output=True, text=True).stdout
T = {}
for l in open('teero_track.txt'):
    k, a, b = l.split(); T[int(k)] = (float(a), float(b))
kc = tpk
i = 0
for l in out.splitlines():
    m = re.search(r'^in (.*?) \| rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) .*reload (\d+)', l)
    if not m: continue
    i += 1; rt = tpk + i
    px, py, wx, wy = (float(m.group(j)) for j in range(4, 8))
    best = min((k for k in range(kc - 20, kc + 80) if k in T), key=lambda k: (T[k][0] - px) ** 2 + (T[k][1] - py) ** 2)
    kc = best
    if i % ev == 0 or 'fire' in l:
        print(f'rt {rt} pos {px:6.0f} {py:5.0f} v {wx:6.1f} {wy:6.1f} |v| {math.hypot(wx, wy):5.1f} in [{m.group(1)}] hook {m.group(8)} rl {m.group(9)}  k {best} lag {rt - best:+d}')
