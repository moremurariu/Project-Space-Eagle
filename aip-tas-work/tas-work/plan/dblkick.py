#!/usr/bin/env python3
"""dblkick.py DIR [kicks=E1,E2] [from=1000] [to=2500] [vars=21,23] [beam=12000] [cores=4] [fpw=3] [min=0.9]
Double every kick of tas-work/kog_full_best.txt: the shot before it becomes a lob that explodes at the kick's own
explosion point together with it (the shaft double kick, generalized). Speed at turn exits is what pays (+12 px/t
saved 3.6-6.4 ticks per exit, NOTES Oct 8) while approach kicks are worth little - and a lob is just the approach
kick spent later. For each kick (explosion E at X, fired F) with a previous shot P: x_ds from P - 8 with
fireplan=P+G@X,F (fpw: the lob fired within fpw of P, forward pre-fires aimed at X; the kick within fpw of F) to the
gate after the next turn (apex + 30), every variant; lines measured against the run by place; a whole-tick lead is
carried to the finish by carry.py (stage 0 keeps the plan). Writes DIR/log."""
import math, os, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
VS = kw.get('vars', '21,23').split(',')
BEAM, CORES, FPW, MIN = kw.get('beam', '12000'), int(kw.get('cores', 4)), kw.get('fpw', '3'), float(kw.get('min', 0.9))
VAR = {}
for l in open(os.path.join(HERE, 'dschain_variants_rf_mix2.txt')):
    if l.startswith('seed='):
        VAR[l.split()[0][5:]] = l.strip() + (' incjump=1' if l.startswith('seed=21') else '') + ' cellreload=1'
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm=960):
    T, E = {}, []
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(p=(float(a[3]), float(a[4])), v=(float(a[5]), float(a[6])), s=float(a[7]), dead='DEAD' in a, fin='FINISH' in a)
        elif a and a[0] == 'E' and 'fired' in a:
            E.append((int(a[2]), int(a[a.index('fired') + 1]), float(a[4]), float(a[5])))
    return T, E


def apexes(T):
    ts = sorted(t for t in T if t > 1000)
    out = []
    for t in ts:
        w = [T[k]['s'] for k in range(t - 15, t + 16) if k in T]
        if T[t]['s'] == min(w) and T[t]['s'] < 0.85 * max(w) and (not out or t - out[-1] > 20) and t - 20 in T and t + 20 in T:
            (ax, ay), (bx, by) = T[t - 20]['v'], T[t + 20]['v']
            if (ax * bx + ay * by) / ((math.hypot(ax, ay) * math.hypot(bx, by)) or 1) < 0.5:
                out.append(t)
    return out


def lead(line, run, t0, t1):
    """the line's lead over the run by place (run's tick at the line's position - line's tick), last on-path value"""
    A, _ = trace(line, t0 - 30)
    B = {t: v['p'] for t, v in trace(run, t0 - 60)[0].items()}
    m, last = None, None
    for t in sorted(A):
        if t < t0 or A[t]['dead']:
            continue
        p, best = A[t]['p'], None
        for r in (range(t - 30, t + 30) if m is None else range(int(m) - 8, int(m) + 14)):
            if r in B and r + 1 in B:
                a, b = B[r], B[r + 1]
                dx, dy = b[0] - a[0], b[1] - a[1]
                u = max(0.0, min(1.0, ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / (dx * dx + dy * dy or 1e-9)))
                dd = math.hypot(p[0] - a[0] - u * dx, p[1] - a[1] - u * dy)
                if best is None or dd < best[0]:
                    best = (dd, r + u)
        if best is None:
            break
        m = best[1]
        if best[0] < 12 and t <= t1:
            last = (m - t, t)
    return last


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
T, E = trace(run)
FIN = min(t for t in T if T[t]['fin'])
AP = apexes(T)
E = sorted(set(E), key=lambda e: e[1])
lo, hi = int(kw.get('from', 1000)), int(kw.get('to', 2500))
want = set(int(x) for x in kw['kicks'].split(',')) if 'kicks' in kw else None
log('dblkick on %d: %d kicks' % (FIN, len(E)))
for i in range(1, len(E)):
    ex, f, x, y = E[i]
    pe, pf = E[i - 1][0], E[i - 1][1]
    if not lo <= ex <= hi or (want and ex not in want) or pf == f or ex - pf < 26 or pe >= ex - 1:
        continue
    nx = [a for a in AP if a > ex + 15]
    gate = min(nx[0] + 30 if nx else FIN, FIN - 4)
    cut = pf - 8
    pre = os.path.join(d, 'c%d.txt' % cut)
    open(pre, 'w').writelines(open(run).readlines()[:cut + 68])
    plan = '%d+%d@%.0f:%.0f,%d' % (pf, ex - pf + 4, x, y, f)
    jobs = [(os.path.join(d, 'k%d_v%s.txt' % (ex, v)), VAR[v]) for v in VS]

    def xds(j):
        cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + pre, 'gate=rt%d' % gate, 'incforce=0', 'threads=1',
               'beam=' + BEAM, 'verbose=0', 'gatelat=64', 'out=' + j[0], 'fireplan=' + plan, 'fpw=' + FPW, 'kickmin=5'] + j[1].split()
        with open(j[0] + '.log', 'w') as lf:
            subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
        return os.path.exists(j[0]) and 'GATE t' in open(j[0] + '.log').read()
    with ThreadPoolExecutor(CORES) as ex_:
        ok = list(ex_.map(xds, jobs))
    best = None
    for good, (out, _) in zip(ok, jobs):
        if not good:
            log('kick %d (fired %d, lob from %d at %.0f,%.0f) -> gate %d: %s NOGATE' % (ex, f, pf, x, y, gate, os.path.basename(out)))
            continue
        L = lead(out, run, cut + 2, gate)
        # did the line double the kick? (two explosions within 2 ticks near the kick)
        _, El = trace(out, cut)
        dbl = sum(1 for e in El if abs(e[0] - ex) <= 3)
        log('kick %d (fired %d, lob from %d at %.0f,%.0f) -> gate %d: %s lead %s, explosions at the kick %d' %
            (ex, f, pf, x, y, gate, os.path.basename(out), '%.2f at %d' % L if L else '-', dbl))
        if L and (best is None or L[0] > best[0]):
            best = (L[0], L[1], out)
    if best and best[0] >= MIN:
        cd = best[2][:-4] + '_carry'
        log('  carry %s from %d (lead %.2f)' % (os.path.basename(best[2]), best[1] - 4, best[0]))
        subprocess.run(['python3', os.path.join(HERE, 'carry.py'), cd, best[2], str(best[1] - 4), 'cores=%d' % CORES, 'stages=6',
                        'what=double kick at %d (lob from %d onto the kick point)' % (ex, pf)], capture_output=True)
        cl = open(os.path.join(cd, 'log')).read() if os.path.exists(os.path.join(cd, 'log')) else ''
        log('  carry: %s' % ' | '.join(l[9:] for l in cl.strip().splitlines()[-3:]))
        if 'NEW BEST' in cl:
            subprocess.run(['cp', BEST, run])
            T, E2 = trace(run)
            FIN = min(t for t in T if T[t]['fin'])
log('done')
