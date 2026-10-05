# ev.py FILE... : exit tick (first vx>20), vx after the exit (max over 3 ticks), fractional t at x=5671 and x=5900, frozen
import sys
from xl import *
def evf(x, path, start=960):
    L = open(path).readlines()
    x.load(path, start + 68)
    out = x.cmd(['loud'] + ['r ' + ' '.join(l.split()[:7]) for l in L[start + 68:]] + ['quiet'])
    S = [parse_state(l) for l in out if l.startswith('S ')]
    if any(s['frz'] for s in S):
        return None
    ex = next((s for s in S if s['vx'] > 20 and s['rt'] > 1005), None)
    if ex is None:
        return None
    i = S.index(ex)
    vmax = max(s['vx'] for s in S[i:i + 3])
    tg = {}
    for g in (5671, 5900):
        for a, b in zip(S, S[1:]):
            if b['x'] >= g and a['x'] < g:
                tg[g] = a['rt'] + (g - a['x']) / (b['x'] - a['x'])
                break
    return ex['rt'], vmax, ex['vy'], tg.get(5671), tg.get(5900)
if __name__ == '__main__':
    x = XL()
    for p in sys.argv[1:]:
        r = evf(x, p)
        print(p.split('/')[-1], r if r is None else 'exit %d vx %.2f vy %.2f t5671 %s t5900 %s' % (r[0], r[1], r[2], '%.2f' % r[3] if r[3] else None, '%.2f' % r[4] if r[4] else None))
