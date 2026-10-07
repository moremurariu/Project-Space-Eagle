#!/usr/bin/env python3
"""hooktl.py RAW.csv OUT.csv: per-tick hook timeline of Teero from hookdet.py's per-frame detections.
A frame is 'grabbed' if its grab score >= 0.6 and the anchor agrees (within 24 units) with a neighbouring frame's;
'flying' if not grabbed and a chain run >= 90 units leaves the tee (shorter runs are his name tag, aim arrow and weapon above him). Ticks (his track's race ticks) take the frames
whose tick rounds to them; a tick is grabbed if most of its frames are. Output columns: k, state (G/F/-), anchor x y,
and, from his track, the pull angle against his velocity (deg; 90 = sideways, > 90 = has a braking component)."""
import csv, math, sys
import numpy as np
TW = __file__.rsplit('/teero/', 1)[0]

rows = list(csv.DictReader(open(sys.argv[1])))
for r in rows:
    for c in r:
        r[c] = float(r[c])
n = len(rows)
st = []
for i, r in enumerate(rows):
    g = r['grab_frac'] >= 0.6
    if g:
        ok = False
        for j in (i - 1, i + 1):
            if 0 <= j < n and rows[j]['grab_frac'] >= 0.6 and math.hypot(rows[j]['anchor_x'] - r['anchor_x'], rows[j]['anchor_y'] - r['anchor_y']) < 24:
                ok = True
        g = ok
    st.append('G' if g else ('F' if r['run_len'] >= 90 else '-'))
T = {}
for l in open(f'{TW}/teero_track.txt'):
    a = l.split()
    if len(a) >= 3:
        T[int(a[0])] = (float(a[1]), float(a[2]))
by = {}
for r, s in zip(rows, st):
    by.setdefault(int(round(r['k'])), []).append((s, r))
out = open(sys.argv[2], 'w')
out.write('k,state,anchor_x,anchor_y,pull_vs_v_deg,speed\n')
for k in sorted(by):
    fr = by[k]
    ng = sum(1 for s, _ in fr if s == 'G')
    nf = sum(1 for s, _ in fr if s == 'F')
    state = 'G' if ng * 2 > len(fr) else ('F' if nf * 2 > len(fr) else '-')
    ax = ay = ang = sp = ''
    if k - 2 in T and k + 2 in T:
        v = ((T[k + 2][0] - T[k - 2][0]) / 4, (T[k + 2][1] - T[k - 2][1]) / 4)
        sp = f'{math.hypot(*v):.1f}'
    if state == 'G':
        gs = [r for s, r in fr if s == 'G']
        axf = float(np.median([r['anchor_x'] for r in gs]))
        ayf = float(np.median([r['anchor_y'] for r in gs]))
        ax, ay = f'{axf:.0f}', f'{ayf:.0f}'
        if k in T and sp:
            px, py = T[k]
            pull = (axf - px, ayf - py)
            c = (pull[0] * v[0] + pull[1] * v[1]) / (math.hypot(*pull) * math.hypot(*v) + 1e-9)
            ang = f'{math.degrees(math.acos(max(-1, min(1, c)))):.0f}'
    out.write(f'{k},{state},{ax},{ay},{ang},{sp}\n')
out.close()
