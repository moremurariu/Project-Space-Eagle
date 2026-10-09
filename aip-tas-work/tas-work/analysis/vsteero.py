#!/usr/bin/env python3
"""vsteero.py RUN [from_rt]: where Teero is faster, per section of RUN (post-grenade).
Per section (our race ticks): ticks Teero gains (lag growth, by projecting our positions onto teero_track.txt), path
speed (px moved per tick) ours vs his over the matched part of his track, our stored speed |v| and the horizontal
ramp factor (DDNet moves x by vx * ramp(|v|), ramp = 1.4^-((50|v| - 550) / 2000), y unramped), how much of our |v| is
vertical, and the direction key: ticks with dir 0 or against vx while |vx| > 5 in the air (0.95 friction / -1.5 per
tick) ours vs Teero's (video extraction teero/teero_inputs_0-3131.csv, 60 fps, his vx sign from his track), and the
shots (ours fired, his fire events)."""
import csv, math, subprocess, sys
TW = __file__.rsplit('/analysis/', 1)[0]
run = sys.argv[1]
r0 = int(sys.argv[2]) if len(sys.argv) > 2 else 966
T = {}
for l in open(f'{TW}/teero_track.txt'):
    a = l.split()
    if len(a) >= 3:
        T[int(a[0])] = (float(a[1]), float(a[2]))
out = subprocess.run([f'{TW}/../ddnet/build-sim/x_trace', f'{TW}/AiP-Gores.map', run, str(r0)], capture_output=True,
                     text=True).stdout
A = {}
for l in out.splitlines():
    a = l.split()
    if len(a) >= 28 and a[0] == 'T':
        # T idx rt x y vx vy |v| hs H hp hx hy in dir jump hook fire tx ty rl R np N gr G j J
        A[int(a[2])] = dict(x=float(a[3]), y=float(a[4]), vx=float(a[5]), vy=float(a[6]), hs=int(a[9]),
                            dir=int(a[14]), jump=int(a[15]), hook=int(a[16]), rl=int(a[21]), np=int(a[23]))
# Teero dir / fire per his race tick (60 fps -> 50 tps)
TD, TF = {}, []
for r in csv.DictReader(open(f'{TW}/teero/teero_inputs_0-3131.csv')):
    try:
        s = float(r['s_since_start'])
    except ValueError:
        continue
    TD[int(round(s * 50))] = int(r['dir'])
    if r['fire'] in ('1', 'True', 'true'):
        TF.append(s * 50)
# lag: project our position onto his track
lag, m = {}, None
for t in sorted(A):
    p, best = (A[t]['x'], A[t]['y']), None
    for r in (range(t - 40, t + 10) if m is None else range(int(m) - 8, int(m) + 12)):
        if r in T and r + 1 in T:
            a, b = T[r], T[r + 1]
            dx, dy = b[0] - a[0], b[1] - a[1]
            u = max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
            d = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
            if best is None or d < best[0]:
                best = (d, r + u)
    if best is None:
        if m is None:
            continue
        break
    m = best[1]
    lag[t] = (t - m, m)


def ramp(v):
    w = v * 50
    return 1.0 if w < 550 else 1.0 / 1.4 ** ((w - 550) / 2000)


secs = [(966, 1100), (1100, 1275), (1275, 1450), (1450, 1575), (1575, 1700), (1700, 1825), (1825, 2000),
        (2000, 2175), (2175, 2350), (2350, 2450), (2450, max(lag))]
print('section     gain | path px/t us  him | |v| ramp vy% | dir-bad us (loss px/t) him | shots us him')
tot = [0, 0, 0]
for s0, s1 in secs:
    ts = [t for t in range(s0, s1) if t in lag and t + 1 in A]
    if len(ts) < 5:
        continue
    gain = lag[ts[-1]][0] - lag[ts[0]][0]
    k0, k1 = lag[ts[0]][1], lag[ts[-1]][1]
    us = sum(math.hypot(A[t + 1]['x'] - A[t]['x'], A[t + 1]['y'] - A[t]['y']) for t in ts) / len(ts)
    ks = [k for k in range(int(k0), int(k1)) if k in T and k + 1 in T]
    him = sum(math.hypot(T[k + 1][0] - T[k][0], T[k + 1][1] - T[k][1]) for k in ks) / max(1, len(ks))
    v = [math.hypot(A[t]['vx'], A[t]['vy']) for t in ts]
    vm = sum(v) / len(v)
    rm = sum(ramp(x) for x in v) / len(v)
    vyf = sum(abs(A[t]['vy']) for t in ts) / max(1e-9, sum(abs(A[t]['vx']) + abs(A[t]['vy']) for t in ts))
    bad = [t for t in ts if abs(A[t]['vx']) > 5 and A[t + 1]['dir'] != (1 if A[t]['vx'] > 0 else -1)]
    loss = sum(abs(A[t]['vx']) - abs(A[t + 1]['vx']) for t in bad
               if A[t + 1]['hs'] != 5 and A[t + 1]['np'] >= A[t]['np']) / len(ts)  # no hook pull, no explosion
    hbad = [k for k in ks if k in TD and abs(T[k + 1][0] - T[k][0]) > 5 and TD[k] != (1 if T[k + 1][0] > T[k][0] else -1)]
    sh_us = sum(1 for t in ts if A[t]['rl'] == 24)
    sh_him = sum(1 for f in TF if k0 <= f < k1)
    print(f'{s0}-{s1}: {gain:+5.1f} | {us:5.1f} {him:5.1f} | {vm:4.1f} {rm:.2f} {100*vyf:3.0f}% | {len(bad):3d}/{len(ts)} ({loss:4.2f}) {len(hbad):3d}/{len(ks)} | {sh_us:2d} {sh_him:2d}')
