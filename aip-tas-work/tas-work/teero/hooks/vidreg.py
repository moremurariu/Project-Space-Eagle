#!/usr/bin/env python3
"""vidreg.py VIDEO T [T...]: fit the camera of Teero's video (screen px per world unit, the tee's screen position) on
frames at video times T, by matching the map's solid tiles (tas-work/map.txt, '#') against the frame's terrain pixels
(brown dirt / green grass). World position of the tee from tas-work/teero_track.txt at race tick (T - 1.383) * 50."""
import subprocess, sys
import numpy as np
TW = __file__.rsplit('/teero/', 1)[0]
T0 = 1.383  # video time of the race start (teero_inputs_0-3131.csv: s_since_start = video_s - 1.383)
W, H = 2560, 1440


def load_map():
    rows = [l.rstrip('\n') for l in open(f'{TW}/map.txt')]
    return np.array([[c == '#' for c in r] for r in rows])


def load_track():
    T = {}
    for l in open(f'{TW}/teero_track.txt'):
        a = l.split()
        if len(a) >= 3:
            T[int(a[0])] = (float(a[1]), float(a[2]))
    return T


def track_at(T, k):
    k0 = int(np.floor(k))
    a, b = np.array(T[k0]), np.array(T[k0 + 1])
    return a + (b - a) * (k - k0)


def frame(video, t):
    raw = subprocess.run(['ffmpeg', '-v', 'error', '-ss', f'{t:.4f}', '-i', video, '-frames:v', '1', '-f', 'rawvideo',
                          '-pix_fmt', 'rgb24', '-'], capture_output=True).stdout
    return np.frombuffer(raw, np.uint8).reshape(H, W, 3).astype(np.int16)


def terrain(img):
    r, g, b = img[..., 0], img[..., 1], img[..., 2]
    brown = (r > 120) & (g > 70) & (g < 140) & (b < 90) & (r > g + 25)
    green = (g > 150) & (b < 90) & (r < 200) & (g > r)
    return brown | green


def fit(M, img, P, s0=1.75):
    ys, xs = np.mgrid[8:H:16, 8:W:16]
    fm = terrain(img)[ys, xs]
    best = None
    for s in np.arange(s0 - 0.25, s0 + 0.26, 0.01):
        for cx in range(W // 2 - 60, W // 2 + 61, 4):
            for cy in range(H // 2 - 60, H // 2 + 61, 4):
                wx = P[0] + (xs - cx) / s
                wy = P[1] + (ys - cy) / s
                tx, ty = np.floor(wx / 32).astype(int), np.floor(wy / 32).astype(int)
                ok = (tx >= 0) & (ty >= 0) & (tx < M.shape[1]) & (ty < M.shape[0])
                mm = np.zeros_like(fm)
                mm[ok] = M[ty[ok], tx[ok]]
                sc = (mm == fm).mean()
                if best is None or sc > best[0]:
                    best = (sc, s, cx, cy)
    return best


if __name__ == '__main__':
    video = sys.argv[1]
    M, T = load_map(), load_track()
    for t in map(float, sys.argv[2:]):
        k = (t - T0) * 50
        P = track_at(T, k)
        sc, s, cx, cy = fit(M, frame(video, t), P)
        print(f'video {t:.3f} k {k:.2f} world {P[0]:.0f} {P[1]:.0f}: agree {sc:.3f} scale {s:.2f} px/unit tee at screen {cx} {cy}')


def register(M, img, P, s=1.75, R=320, ds=4):
    """world position of the screen center: FFT cross-correlation of the frame's terrain mask with the map's solid
    mask around the guess P (+-R world units); returns (cx_world, cy_world, peak score)"""
    h, w = H // ds, W // ds
    fm = terrain(img)[ds // 2::ds, ds // 2::ds][:h, :w].astype(np.float32)
    # map window in the same pixel scale (s / ds px per unit), R units of margin
    pu = s / ds
    x0, y0 = P[0] - (W / 2) / s - R, P[1] - (H / 2) / s - R
    mw, mh = int(w + 2 * R * pu), int(h + 2 * R * pu)
    ys, xs = np.mgrid[0:mh, 0:mw]
    wx, wy = x0 + (xs + 0.5) / pu, y0 + (ys + 0.5) / pu
    tx, ty = np.floor(wx / 32).astype(int), np.floor(wy / 32).astype(int)
    ok = (tx >= 0) & (ty >= 0) & (tx < M.shape[1]) & (ty < M.shape[0])
    mm = np.zeros((mh, mw), np.float32)
    mm[ok] = M[ty[ok], tx[ok]]
    a = fm - fm.mean()
    b = mm - mm.mean()
    F = np.fft.rfft2(b) * np.conj(np.fft.rfft2(a, s=b.shape))
    c = np.fft.irfft2(F, s=b.shape)
    c = c[: mh - h + 1, : mw - w + 1]  # valid offsets
    oy, ox = np.unravel_index(np.argmax(c), c.shape)
    # frame pixel (0,0) sits at map-window pixel (ox, oy): screen center in world units
    cxw = x0 + (ox + w / 2) / pu
    cyw = y0 + (oy + h / 2) / pu
    return cxw, cyw, float(c[oy, ox] / (np.linalg.norm(a) * np.linalg.norm(b) + 1e-9))
