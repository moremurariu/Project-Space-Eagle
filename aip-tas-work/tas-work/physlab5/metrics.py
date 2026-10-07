# metrics.py FILE...: pickup rt, first y<=2290 climbing (vy<0, grenade), exit kick (largest vx jump), t5671, t5900, frozen
import sys
from xl import *
for path in sys.argv[1:]:
    x = XL()
    L = open(path).readlines()
    x.load(path, 900 + 68)
    seq = [tuple(int(v) for v in l.split()[:6]) for l in L[968:]]
    # use exact weapon from file: xlab 'r' command
    out = x.cmd(['loud'] + ['r ' + ' '.join(l.split()[:7]) for l in L[968:]] + ['quiet'])
    S = [parse_state(l) for l in out if l.startswith('S ')]
    pick = next((s['rt'] for s in S if s['gren']), None)
    y2290 = next((s['rt'] for s in S if s['gren'] and s['y'] <= 2290 and s['vy'] < 0 and s['x'] < 5400), None)
    best = None
    for a, b in zip(S, S[1:]):
        dv = b['vx'] - a['vx']
        if best is None or dv > best[0]:
            best = (dv, b)
    ex = best[1]
    t = {}
    for g in (5671, 5900):
        t[g] = next((s['rt'] for s in S if s['x'] >= g), None)
    frz = next((s['rt'] for s in S if s['frz']), None)
    print(f"{path.split('/')[-1]:40s} lines {len(L)} end rt {len(L)-68} pickup {pick} y<=2290 {y2290} exit {ex['rt']} v ({ex['vx']:.2f},{ex['vy']:.2f}) "
          f"x>=5671 {t[5671]} x>=5900 {t[5900]} frz {frz}")
    x.close()
