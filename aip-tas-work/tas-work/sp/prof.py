import sys, math
from trk import *
run = sys.argv[1]
step = int(sys.argv[2]) if len(sys.argv) > 2 else 25
T, E = trace(run)
R = track()
# our run's own track for teero lag
first = min(t for t in T if T[t]['p'][1] < 2400 and t > 980)
L = lagmap({t: T[t] for t in T if t >= 985}, R, 995)
def tsp(k, w=6):
    k = int(round(k))
    if k - w // 2 in R and k + w // 2 in R:
        a, b = R[k - w // 2], R[k + w // 2]
        return math.hypot(b[0] - a[0], b[1] - a[1]) / w
def osp(t, w=6):
    if t - w // 2 in T and t + w // 2 in T:
        a, b = T[t - w // 2]['p'], T[t + w // 2]['p']
        return math.hypot(b[0] - a[0], b[1] - a[1]) / w
print(' rt   label  lag   lat | |v|  disp  Tdisp | kicks hook%')
for t in range(990, max(T), step):
    if t not in L: continue
    lab, lat = L[t][1], L[t][0]
    ks = ' '.join('%d:%.0f/%.1f' % (u, e['f'], e['cos']) for u in range(t, t + step) for e in E.get(u, []))
    hk = sum(1 for u in range(t, t + step) if u in T and T[u]['hs'] in (3, 4, 5) ) 
    print('%4d %7.1f %5.1f %5.0f | %5.1f %5.1f %5.1f | %s h%d' % (t, lab, t - lab, lat, T[t]['s'], osp(t) or 0, tsp(lab) or 0, ks, hk))
