"""Shared helpers: load x_trace output, Teero's track, project a run onto a reference line (lag)."""
import math, subprocess, os
TW = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = TW + '/../ddnet/build-sim'
MAP = TW + '/AiP-Gores.map'

def trace(run, frm=900):
    out = subprocess.run([BIN + '/x_trace', MAP, run, str(frm)], capture_output=True, text=True).stdout
    T, E = {}, {}
    for l in out.splitlines():
        a = l.split()
        if a and a[0] == 'T':
            T[int(a[2])] = dict(p=(float(a[3]), float(a[4])), v=(float(a[5]), float(a[6])), s=float(a[7]), hs=int(a[9]),
                                hp=(float(a[11]), float(a[12])), dir=int(a[14]), jump=int(a[15]), hook=int(a[16]),
                                fire=int(a[17]), tx=int(a[18]), ty=int(a[19]), rl=int(a[21]), np=int(a[23]), gr=int(a[25]), j=int(a[27]),
                                dead='DEAD' in a, fin='FINISH' in a)
        elif a and a[0] == 'E':
            E.setdefault(int(a[2]), []).append(dict(ex=(float(a[4]), float(a[5])), dist=float(a[a.index('dist') + 1]),
                                                     f=float(a[a.index('|f|') + 1]), cos=float(a[a.index('cos') + 1]),
                                                     dv2=float(a[a.index('dv2') + 1])))
    return T, E

def track(path=TW + '/teero_track.txt'):
    R = {}
    for l in open(path):
        a = l.split()
        if len(a) >= 3:
            R[int(float(a[0]))] = (float(a[1]), float(a[2]))
    return R

def project(R, p, hint, lo=8, hi=14):
    best = None
    for r in range(int(hint) - lo, int(hint) + hi):
        if r in R and r + 1 in R:
            a, b = R[r], R[r + 1]
            dx, dy = b[0] - a[0], b[1] - a[1]
            u = max(0, min(1, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
            d = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
            if best is None or d < best[0]:
                best = (d, r + u)
    return best

def lagmap(T, R, hint0):
    """per race tick of run T: (label on R, lateral distance)"""
    L, m = {}, hint0
    for t in sorted(T):
        b = project(R, T[t]['p'], m)
        if b is None:
            continue
        m = b[1]
        L[t] = b
    return L
