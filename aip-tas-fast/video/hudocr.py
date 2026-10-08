"""HUD reader for the Teero video: position / speed / angle values (bottom-right box) per frame."""
import numpy as np

LINES = [(53, 77), (91, 115), (168, 192), (207, 231), (283, 307)]  # pos x, pos y, speed x, speed y, angle (crop rows)
X0, Y0 = 2260, 1030  # crop origin in the 2560x1440 frame


def textmask(crop):
    crop = crop.astype(int)
    R, G, B = crop[..., 0], crop[..., 1], crop[..., 2]
    white = crop.min(-1) > 190
    green = (G > 170) & (R < 150) & (B < 150)
    red = (R > 190) & (R - G > 50) & (R - B > 50)
    return white | green | red


def chars(mask_line):
    """split a line mask (only the value area, x >= 150) into character boxes by column gaps"""
    cols = mask_line.sum(0)
    out = []
    x = 0
    W = len(cols)
    while x < W:
        if cols[x] > 0:
            s = x
            while x < W and cols[x] > 0:
                x += 1
            sub = mask_line[:, s:x]
            ys = np.nonzero(sub.sum(1))[0]
            out.append((s, x, ys.min(), ys.max() + 1, sub[ys.min():ys.max() + 1]))
        else:
            x += 1
    return out


def norm(bm, h=20, w=12):
    """resize a character bitmap to h x w (nearest)"""
    H, Wd = bm.shape
    yi = (np.arange(h) * H / h).astype(int)
    xi = (np.arange(w) * Wd / w).astype(int)
    return bm[yi][:, xi].astype(float)


def classify(c, templates):
    s, e, y0, y1, bm = c
    hgt = y1 - y0
    wid = e - s
    if hgt <= 4 and 9 <= y0 <= 16 and wid >= 4 and bm.sum() >= 6:
        return '-'
    if hgt < 15:
        return ''  # dots and specks: the decimal point is restored from the fixed 2 decimals
    best, bk = 1e9, '?'
    v = norm(bm)
    for k, ts in templates.items():
        for t in ts:
            d = np.abs(v - t).sum() + 3 * abs(wid - t.shape[1] if False else 0)
            if d < best:
                best, bk = d, k
    return bk


def read_values(frame, templates):
    crop = frame[Y0:Y0 + 320, X0:X0 + 300]
    m = textmask(crop)
    vals = []
    for (a, b) in LINES:
        cs = chars(m[a:b, 150:])
        s = ''.join(classify(c, templates) for c in cs)
        vals.append(s)
    return vals


def to_value(s):
    """sign + digits -> float with 2 decimals (the HUD always prints 2)"""
    neg = s.startswith('-')
    d = s.lstrip('-')
    if not d.isdigit() or len(d) < 3:
        return None
    return (-1 if neg else 1) * int(d) / 100.0
