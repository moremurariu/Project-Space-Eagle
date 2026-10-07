#!/usr/bin/env python3
"""hookdet.py VIDEO T0 T1 OUT.csv: Teero's hook chain per frame of his video (60 fps) from video time T0 to T1.
The camera is centered on Teero (user); 1.75 screen px per world unit and the video-to-track timing (his track's race
tick = (t - 1.383) * 50 - 2.6) come from vidreg.py. A chain is a straight dark, neutral-gray line from the screen
center. Per direction (0.5 deg): the map distance D to the first solid tile (a grabbed hook ends there, <= 380 units),
the fraction of chain-like pixels on [R0, D] (grab score) and the chain-like run length from R0 (flying hooks).
Columns: t, k, x, y, grab_dir, grab_frac, grab_dist, anchor_x, anchor_y, run_dir, run_len."""
import os, subprocess, sys
import numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import vidreg as V

S = 1.75
CX, CY = V.W / 2, V.H / 2
NA = 720
R0 = 44  # world units: inside this the tee, its weapon and point-blank explosions cover the chain
RS = np.arange(R0, 381, 2.0)
ANG = 2 * np.pi * np.arange(NA) / NA
DX, DY = np.cos(ANG), np.sin(ANG)


def chainlike(img):
    r, g, b = img[..., 0].astype(np.int16), img[..., 1].astype(np.int16), img[..., 2].astype(np.int16)
    mx = np.maximum(np.maximum(r, g), b)
    mn = np.minimum(np.minimum(r, g), b)
    return (mx < 110) & (mx - mn < 18) & (b - r < 12)


def map_dist(M, P):
    xs = P[0] + DX[:, None] * RS[None, :]
    ys = P[1] + DY[:, None] * RS[None, :]
    tx, ty = np.floor(xs / 32).astype(int), np.floor(ys / 32).astype(int)
    ok = (tx >= 0) & (ty >= 0) & (tx < M.shape[1]) & (ty < M.shape[0])
    sol = np.zeros_like(ok)
    sol[ok] = M[ty[ok], tx[ok]]
    hit = sol.any(axis=1)
    first = np.argmax(sol, axis=1)
    return np.where(hit, RS[first], 0.0)


def profile(img, D):
    C = chainlike(img)
    best = np.zeros((NA, len(RS)), bool)
    for off in (-3.0, 0.0, 3.0):
        xs = np.clip((CX + DX[:, None] * RS[None, :] * S - DY[:, None] * off).astype(int), 0, V.W - 1)
        ys = np.clip((CY + DY[:, None] * RS[None, :] * S + DX[:, None] * off).astype(int), 0, V.H - 1)
        best |= C[ys, xs]
    n = np.clip(((D - R0) / 2).astype(int), 0, len(RS))
    cs = np.cumsum(best, axis=1)
    # a grabbed chain is >= 40 units long past R0 (shorter segments are the tee's weapon / explosion sprites)
    frac = np.where((D > 0) & (D - R0 >= 40), cs[np.arange(NA), np.maximum(n - 1, 0)] / np.maximum(n, 1), 0.0)
    # run from R0 with gaps up to 3 samples
    miss = ~best
    gaps = np.zeros(NA, int)
    run = np.zeros(NA)
    alive = np.ones(NA, bool)
    for j in range(len(RS)):
        gaps = np.where(miss[:, j], gaps + 1, 0)
        alive &= gaps <= 3
        run = np.where(alive & best[:, j], RS[j], run)
    return frac, run


def main():
    video, t0, t1, out = sys.argv[1], float(sys.argv[2]), float(sys.argv[3]), sys.argv[4]
    M, T = V.load_map(), V.load_track()
    p = subprocess.Popen(['ffmpeg', '-v', 'error', '-ss', f'{t0:.4f}', '-i', video, '-t', f'{t1 - t0:.4f}', '-f',
                          'rawvideo', '-pix_fmt', 'rgb24', '-'], stdout=subprocess.PIPE)
    fs = V.W * V.H * 3
    f = open(out, 'w')
    f.write('t,k,x,y,grab_dir,grab_frac,grab_dist,anchor_x,anchor_y,run_dir,run_len\n')
    i = 0
    while True:
        buf = p.stdout.read(fs)
        if len(buf) < fs:
            break
        t = t0 + i / 60.0
        i += 1
        k = (t - V.T0) * 50 - 2.6
        P = V.track_at(T, k)
        img = np.frombuffer(buf, np.uint8).reshape(V.H, V.W, 3)
        D = map_dist(M, P)
        frac, run = profile(img, D)
        g = int(np.argmax(frac))
        r = int(np.argmax(run))
        ax, ay = P[0] + DX[g] * D[g], P[1] + DY[g] * D[g]
        f.write(f'{t:.4f},{k:.2f},{P[0]:.1f},{P[1]:.1f},{g * 0.5:.1f},{frac[g]:.3f},{D[g]:.0f},{ax:.1f},{ay:.1f},'
                f'{r * 0.5:.1f},{run[r]:.0f}\n')
        if i % 100 == 0:
            f.flush()
            print(f'{i} frames, t {t:.2f}', flush=True)
    f.close()


if __name__ == '__main__':
    main()
