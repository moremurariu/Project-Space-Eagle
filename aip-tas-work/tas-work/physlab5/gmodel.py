"""Analytic grenade model (speed 1000, curvature 7 -> 20 px per tick, drop 0.28 n^2) for the L-arm right-face double kick.
The projectile of a shot fired on input step m starts at the tee position before that step + 21*dir; its n-th segment is
simulated during step m+n-1 (kick applied with the tee position before that step)."""
import math

FACE_X = 5248.0
FACE_Y0, FACE_Y1 = 1920.0, 1952.0


def face_hits(x0, y0, n, samples=24):
    """aims (deg, y down) whose grenade from a tee at (x0, y0) crosses the right face x=5248 during segment n.
    returns list of (aim_deg, y_cross)"""
    if x0 <= FACE_X + 22:
        return []
    out = []
    # crossing parameter tau in (n-1, n]: c = (FACE_X - x0) / (20 tau + 21)
    c_hi = (FACE_X - x0) / (20 * (n - 1) + 21)   # tau = n-1 (less negative)
    c_lo = (FACE_X - x0) / (20 * n + 21)         # tau = n
    for k in range(samples + 1):
        c = c_lo + (c_hi - c_lo) * (k + 0.5) / (samples + 1)
        if c <= -1:
            continue
        s = -math.sqrt(1 - c * c)
        tau = (FACE_X - x0 - 21 * c) / (20 * c)
        y = y0 + 21 * s + 20 * tau * s + 0.28 * tau * tau
        if FACE_Y0 + 1 <= y <= FACE_Y1 - 1:
            out.append((math.degrees(math.atan2(s, c)) % 360, y))
    return out


def double_kick_T(S, nmin=26, nmax=34, zone=(5281, 5300, 1915, 1965), kickr=46):
    """earliest explosion step e (index into S) of a feasible pre-fire (fired at step m with reload 0 / grenade
    active at S[m-1]) whose face explosion lands within kickr of the tee at S[e-1], tee in zone at S[e-1].
    returns (e, m, n, aim, y_cross) or None"""
    best = None
    for m in range(1, len(S)):
        p = S[m - 1]
        if p['reload'] != 0 or p['gren'] != 2 or p['frz']:
            continue
        if not (2200 <= p['y'] <= 2340):
            continue
        for n in range(nmin, nmax + 1):
            e = m + n - 1
            if e >= len(S):
                break
            if best is not None and e >= best[0]:
                break
            t = S[e - 1]
            if not (zone[0] <= t['x'] <= zone[1] and zone[2] <= t['y'] <= zone[3]):
                continue
            for a, yc in face_hits(p['x'], p['y'], n):
                if math.hypot(t['x'] - FACE_X, t['y'] - yc) <= kickr:
                    best = (e, m, n, a, yc)
                    break
            if best is not None and best[0] == e:
                break
    return best
