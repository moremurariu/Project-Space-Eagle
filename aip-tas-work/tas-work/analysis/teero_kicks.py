#!/usr/bin/env python3
"""teero_kicks.py VIDEO T0 T1 > scan.txt: Teero's explosions in his video (orange sprite blobs per frame) and their distance
to the screen centre, i.e. to him (the camera follows him; 0.875 px per world unit at the half resolution used here).
Onsets are the NEW rows with area >= 2000 and dist < 200. Needs ffmpeg, numpy, scipy."""
# (the camera follows him). Half resolution: 0.875 screen px per world unit, center (640, 360).
import subprocess, sys, numpy as np
from scipy import ndimage
V = sys.argv[1]; t0, t1 = float(sys.argv[2]), float(sys.argv[3])
W, H, SC = 1280, 720, 0.875
p = subprocess.Popen(['ffmpeg', '-v', 'error', '-ss', str(t0), '-t', str(t1 - t0), '-i', V, '-vf', f'scale={W}:{H}',
                      '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-'], stdout=subprocess.PIPE)
n = 0; prev = []
fr0 = int(round(t0 * 60))
while True:
    raw = p.stdout.read(W * H * 3)
    if len(raw) < W * H * 3: break
    a = np.frombuffer(raw, np.uint8).reshape(H, W, 3).astype(np.int16)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    m = (r > 220) & (g > 60) & (g < 175) & (b < 90)
    lab, k = ndimage.label(ndimage.binary_dilation(m, iterations=3))
    blobs = []
    if k:
        idx = np.arange(1, k + 1)
        areas = ndimage.sum(m, lab, idx)
        cents = ndimage.center_of_mass(m, lab, idx)
        for ar, (cy, cx) in zip(areas, cents):
            if ar >= 150: blobs.append((cx, cy, ar))
    fr = fr0 + n
    for cx, cy, ar in blobs:
        new = all(abs(cx - px) + abs(cy - py) > 80 for px, py, _ in prev)
        d = ((cx - W / 2) ** 2 + (cy - H / 2) ** 2) ** 0.5 / SC
        print(f'{fr} {"NEW" if new else "old"} cx {cx:.0f} cy {cy:.0f} area {ar:.0f} dist {d:.1f}')
    prev = blobs; n += 1
