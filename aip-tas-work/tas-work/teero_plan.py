#!/usr/bin/env python3
"""Shot plan (seg plan=) from Teero's fire times (teero/teero_inputs_0-3131.csv, audio) paired with his explosions
(teero/catalog/shots.tsv), shifted into our race-tick frame by a per-shot lag measured on a reference run.
usage: teero_plan.py REFRUN RT0 RT1 OUT [win=2] [r=48] [tol=2] [pb=free|target]
  every Teero fire whose shifted tick lies in [RT0, RT1] becomes an entry; point-blank shots (explosion within 3
  ticks of the fire) -> 'free' window (or a target entry with pb=target), pre-fires -> target entry with the
  explosion point and race tick."""
import csv, math, os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
ref, rt0, rt1, outp = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
a = dict(x.split('=', 1) for x in sys.argv[5:])
WIN = int(a.get('win', 2)); R = float(a.get('r', 48)); TOL = int(a.get('tol', 2)); PB = a.get('pb', 'free')
T = {}
for l in open('teero_track.txt'):
    k, x, y = l.split(); T[int(k)] = (float(x), float(y))
rows = list(csv.DictReader(open('teero/teero_inputs_0-3131.csv')))
fires = [float(r['s_since_start']) * 50 - 3.2 for r in rows if r['fire'] == '1']
cat = list(csv.DictReader(open('teero/catalog/shots.tsv'), delimiter='\t'))
expl = [(float(c['race_tick']) - 3, float(c['expl_tile_x']) * 32, float(c['expl_tile_y']) * 32) for c in cat]
# pairing: the exit double kick (fires 1+2 -> explosion 1), then in order, each explosion to the latest fire at or
# before it (+2 ticks of timing slack) that is not yet used
pairs = {0: 0, 1: 0}
fi = 2
for ei in range(1, len(expl)):
    t = expl[ei][0]
    while fi + 1 < len(fires) and fires[fi + 1] <= t + 2:
        fi += 1
    if fi < len(fires) and fires[fi] <= t + 2 and fi not in pairs:
        pairs[fi] = ei
        fi += 1
# our lag at each of his positions, from the reference run (nearest track point search as in lagtrack.py), or a
# constant lag (lag=L, ref ignored)
CONST = int(a['lag']) if 'lag' in a else (0 if a.get('ref') == '1' else None)
out = '' if CONST is not None else subprocess.run(['../ddnet/build-sim/lab', 'AiP-Gores.map', 'replay ' + ref], capture_output=True, text=True).stdout
X = {}
for l in out.splitlines():
    m = re.search(r'rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+)', l)
    if m and l.startswith('in ') and int(m.group(1)) not in X:
        X[int(m.group(1))] = (float(m.group(2)), float(m.group(3)))
ks = sorted(T); kc = None; ourrt = {}
for rt in sorted(X):
    if rt < 0: continue
    x, y = X[rt]
    lo = (kc - 30) if kc is not None else max(ks[0], rt - 150)
    best = min((k for k in range(lo, lo + 180) if k in T), key=lambda k: (T[k][0] - x) ** 2 + (T[k][1] - y) ** 2)
    kc = best
    ourrt.setdefault(best, rt)
def lag_at(true_t):
    if CONST is not None: return CONST
    k = round(true_t + 3)
    for d in range(0, 20):
        for kk in (k - d, k + d):
            if kk in ourrt: return ourrt[kk] - (kk - 3)
    return 0
lines = [f'# from Teero fires/explosions, ref {ref}, window {rt0}-{rt1}']
for fi, f in enumerate(fires):
    if fi not in pairs: continue
    te, ex, ey = expl[pairs[fi]]
    if a.get('ref') == '1':
        # position-indexed plan (seg planref=1): window in Teero track ticks where he fired, te = flight time
        L = 3; ft = round(f + 3); et = max(0, round(te - f))
        if ft < rt0 or ft > rt1: continue
        if te - f <= 3 and PB == 'free':
            lines.append(f'{ft - WIN} {ft + WIN} free   # fire {fi + 1} at {f:.1f} (track {ft}) point-blank')
        else:
            lines.append(f'{ft - WIN} {ft + WIN} {ex:.0f} {ey:.0f} {R:.0f} {et} {TOL}   # fire {fi + 1} at {f:.1f} (track {ft}) -> +{te - f:.1f} at {ex:.0f},{ey:.0f}')
        continue
    L = lag_at(f)
    ft = round(f + L); et = round(te + L)
    if ft < rt0 or ft > rt1: continue
    if te - f <= 3 and PB == 'free':
        lines.append(f'{ft - WIN} {ft + WIN} free   # fire {fi + 1} at {f:.1f} (+{L}) point-blank, expl {te:.1f}')
    else:
        lines.append(f'{ft - WIN} {ft + WIN} {ex:.0f} {ey:.0f} {R:.0f} {et} {TOL}   # fire {fi + 1} at {f:.1f} (+{L}) -> expl {te:.1f} at {ex:.0f},{ey:.0f}')
open(outp, 'w').write('\n'.join(lines) + '\n')
print('\n'.join(lines))
