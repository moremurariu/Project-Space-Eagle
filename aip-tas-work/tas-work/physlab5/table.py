import sys
from xl import *
x = XL()
for path in sys.argv[1:]:
    L = open(path).readlines()
    x.load(path, 0)
    out = x.cmd(['loud'] + ['r ' + ' '.join(l.split()[:7]) for l in L] + ['quiet'])
    S = [parse_state(l) for l in out if l.startswith('S ')]
    fires = [S[i]['rt'] for i in range(len(S)) if L[i].split()[3] == '1' and S[i]['proj'] > (S[i-1]['proj'] if i else 0)]
    pick = next(s['rt'] for s in S if s['gren'])
    y2290 = next(s['rt'] for s in S if s['gren'] and s['y'] <= 2290 and s['vy'] < 0)
    kicks = [(b['rt'], b['vx'] - a['vx'], b['vx'], b['vy']) for a, b in zip(S, S[1:]) if b['rt'] > 990 and b['vx'] - a['vx'] > 5]
    def tg(g):
        for a, b in zip(S, S[1:]):
            if b['x'] >= g > a['x'] and b['rt'] > 1000:
                return b['rt'], a['rt'] + (g - a['x']) / (b['x'] - a['x'])
        return None, None
    t1, f1 = tg(5671); t2, f2 = tg(5900)
    k = '; '.join(f'{r}: +{d:.1f} -> ({vx:.2f},{vy:.2f})' for r, d, vx, vy in kicks[:2])
    print(f"{path.split('/')[-1]:28s} pick {pick} y<=2290 {y2290} shots(rt) {fires} kicks {k} | x>=5671 {t1} ({f1}) x>=5900 {t2} frz {any(s['frz'] for s in S)}")
