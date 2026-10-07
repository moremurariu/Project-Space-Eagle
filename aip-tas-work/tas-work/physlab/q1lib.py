from seqopt import *
RT0 = 297   # race tick of the first continuation input
def cross(S, ygate=1000, xmin=-1e9, xmax=1e9):
    py = None
    for i, s in enumerate(S):
        if s['frz']:
            return None, i
        if s['y'] >= ygate and xmin <= s['x'] <= xmax:
            yp = S[i - 1]['y'] if i else s['y'] - s['vy']
            f = (ygate - yp) / max(1e-6, s['y'] - yp)
            return RT0 + i - 1 + f, i
    return None, len(S)

def make_obj(ygate=1000, lam=0.05, xmin=-1e9, xmax=1e9):
    def obj(S):
        t, i = cross(S, ygate, xmin, xmax)
        if t is None:
            ymax = max([s['y'] for s in S[:i]] or [0])
            return 1000 + (ygate - ymax) / 10 + (50 if i < len(S) else 0)
        return t - lam * E(S[i])
    return obj
