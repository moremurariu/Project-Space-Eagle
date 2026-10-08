"""per-tick states from the HUD: frame f = game time f*50/60 ticks; positions, speeds (median-cleaned), angle.
usage: ticks.py hud.csv out.csv"""
import csv, sys
import numpy as np

rows = list(csv.DictReader(open(sys.argv[1])))
N = len(rows)
cols = ['px', 'py', 'vx', 'vy', 'angle']
V = {c: np.full(N, np.nan) for c in cols}
for i, r in enumerate(rows):
    for c in cols:
        if r[c] != '':
            V[c][i] = float(r[c])
# clean isolated OCR errors: compare with the median of the 5 neighbours (excluding itself); replace outliers
tol = {'px': 0.5, 'py': 0.5, 'vx': 3.0, 'vy': 3.0, 'angle': 30.0}
fixed = 0
for c in cols:
    x = V[c].copy()
    for i in range(N):
        nb = [x[j] for j in range(max(0, i - 3), min(N, i + 4)) if j != i and not np.isnan(x[j])]
        if not nb:
            continue
        med = np.median(nb)
        lim = tol[c] * (1 + (c == 'vy') * 2) if c != 'angle' else 30
        if np.isnan(x[i]) or abs(x[i] - med) > lim:
            # linear interpolation from the nearest good neighbours
            lo = next((j for j in range(i - 1, -1, -1) if not np.isnan(x[j]) and abs(x[j] - med) <= lim), None)
            hi = next((j for j in range(i + 1, N) if not np.isnan(x[j]) and abs(x[j] - med) <= lim), None)
            if lo is not None and hi is not None:
                V[c][i] = x[lo] + (x[hi] - x[lo]) * (i - lo) / (hi - lo)
                fixed += 1
print('fixed values', fixed)
F = np.arange(N)
t = F * 50 / 60
k = np.floor(t + 1e-9).astype(int)
a = t - k
n = k.max() + 2
A = np.zeros((N, n))
A[F, k] = 1 - a
A[F, k + 1] += a
out = {}
for c, scale in (('px', 32), ('py', 32), ('vx', 32 / 50), ('vy', 32 / 50)):
    s, *_ = np.linalg.lstsq(A, V[c] * scale, rcond=None)
    r = A @ s - V[c] * scale
    print(c, 'rms residual', np.sqrt(np.mean(r ** 2)), 'max', np.abs(r).max(), 'at frame', int(np.abs(r).argmax()))
    out[c] = s
# angle: per tick, the frame closest in time
ang = np.array([V['angle'][min(N - 1, int(round(kk * 60 / 50)))] for kk in range(n)])
with open(sys.argv[2], 'w') as f:
    f.write('tick,x,y,vxd,vy,angle,xr,yr\n')
    for kk in range(n - 1):
        f.write(f"{kk},{out['px'][kk]:.2f},{out['py'][kk]:.2f},{out['vx'][kk]:.3f},{out['vy'][kk]:.3f},{ang[kk]:.2f},"
                f"{int(round(out['px'][kk]))},{int(round(out['py'][kk]))}\n")
fr = [np.abs(out[c] - np.round(out[c])) for c in ('px', 'py')]
print('mean |frac| of tick positions', fr[0][200:].mean(), fr[1][200:].mean())
