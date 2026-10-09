#!/usr/bin/env python3
"""kicktabu.py DIR [kicks=E1,..] [from=1020] [to=2540] [ws=1,6] [cores=4] [vars=dschain_variants_rf_mix.txt] [chain=3]
Structure search by kick tabu: x_ds keeps the run's shot structure when it is allowed to (the run's kicks look best
at every intermediate step, so a plan that spends the grenade elsewhere - Teero's U-turn 1 - is pruned before it pays
off), and finds the other structure once the run's own kick is forbidden. So every kick of tas-work/kog_full_best.txt
(explosion E, fired F) is forbidden in turn: x_ds re-searches from F - 6 (the shot not fired yet) with no explosion at
all over E-w..E+w (x_ds nokick + nokickall; w = 1: the kick moves by >= 2 ticks, w = 6: it moves or is dropped and the
slot is spent elsewhere) to the gate after the next turn (apex + 30), every variant; the best line is spliced onto the
run where it is the run's exact state some ticks earlier (multigraft) or, a fraction of a tick ahead, re-searched on
through the next turns (chain); a server-checked gain is committed and pushed. Writes DIR/log."""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
CORES = int(kw.get('cores', 4))
WS = [int(x) for x in kw.get('ws', '1,6').split(',')]
CHAIN = int(kw.get('chain', 3))
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm=990):
    T, E = {}, []
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(v=(float(a[5]), float(a[6])), s=float(a[7]), fin='FINISH' in a)
        elif a and a[0] == 'E' and 'fired' in a:
            E.append((int(a[2]), int(a[a.index('fired') + 1])))
    return T, E


def finish(path):
    T, _ = trace(path, 2400)
    f = [t for t in T if T[t]['fin']]
    return min(f) if f else None


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


def xds(run, prefix, gate, out, var, fin, extra):
    g = 'gate=finish' if gate >= fin - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=4000',
           'verbose=0', 'out=' + out] + var.split() + extra
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def publish(path, ticks, what):
    cur = finish(BEST)
    if cur is not None and ticks >= cur:
        return False
    chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), path], capture_output=True, text=True).stdout
    m = re.search(r'-> (\d+) ticks', chk)
    if not (m and int(m.group(1)) == ticks and 'frozen ticks 0' in chk):
        log('  %d failed the server check' % ticks)
        return False
    dst = os.path.join(TW, 'kog_full_%d.txt' % ticks)
    subprocess.run(['cp', path, dst])
    subprocess.run(['cp', path, BEST])
    with open(os.path.join(TW, 'NOTES.md'), 'a') as nf:
        nf.write('- kicktabu %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): kicktabu %s (the run\'s kick forbidden, x_ds re-search + splice)\n\n'
           'Server-checked: TasReplay, no freeze, no double start.\n\nCo-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\n'
           'Claude-Session: https://claude.ai/code/session_01XxupAVqSvwb6MWzmsVwkue') % (ticks, ticks / 50.0, what)
    subprocess.run(['git', '-C', ROOT, 'add', os.path.relpath(dst, ROOT), 'tas-work/kog_full_best.txt', 'tas-work/NOTES.md'])
    subprocess.run(['git', '-C', ROOT, 'commit', '-q', '-m', msg])
    for k in range(4):
        if subprocess.run(['git', '-C', ROOT, 'push', '-u', 'origin', 'claude/sweet-maxwell-3t11yw']).returncode == 0:
            break
        time.sleep(2 ** (k + 1))
    log('  NEW BEST %d committed and pushed' % ticks)
    return True


def graft(line, run, lo, hi, G, FIN, what):
    gd = line + '_mg'
    mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), line, run, gd, str(lo), str(min(hi, FIN - 5)), '6', '3', str(CORES)],
                        capture_output=True, text=True).stdout
    log('  multigraft %s: %s' % (os.path.basename(line), ' | '.join(mg.strip().splitlines()[-2:])))
    cs = [os.path.join(gd, g) for g in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
    if G >= FIN - 3:
        cs.append(line)
    best = None
    for p in cs:
        f = finish(p)
        if f is not None and (best is None or f < best[0]):
            best = (f, p)
    return bool(best and best[0] < FIN and publish(best[1], best[0], what))


def try_kick(E, F):
    run = os.path.join(d, 'run_%d.txt' % E)
    subprocess.run(['cp', BEST, run])
    FIN = finish(run)
    T, _ = trace(run)
    ap = apexes(T)
    nx = [a for a in ap if a > E + 20]
    G = min(max(nx[0] + 30 if nx else FIN, E + 60), FIN)
    cut = F - 6
    pre = os.path.join(d, 'k%d_p.txt' % E)
    open(pre, 'w').writelines(open(run).readlines()[:cut + 68])
    jobs = []
    for w in WS:
        for i, v in enumerate(VAR):
            jobs.append(('%s/k%d_w%d_v%d.txt' % (d, E, w, i), v, ['nokick=%d,%d' % (E - w, E + w), 'nokickall=1'], w))
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda j: (xds(run, pre, G, j[0], j[1], FIN, j[2]), j[0], j[3]), jobs))
    ok = sorted(r for r in res if r[0] is not None)
    g = min(G, FIN)
    log('kick %d (fired %d, run %d): cut %d, gate %d: %s' % (E, F, FIN, cut, g, ' '.join('%.2f(w%d)' % (r[0], r[2]) for r in ok[:8]) or 'NOGATE'))
    for t, line, w in ok[:2]:
        if t > g - 0.9:
            break
        if graft(line, run, cut + 3, G, G, FIN, 'kick %d (fired %d) forbidden over %d-%d' % (E, F, E - w, E + w)):
            return True
    if ok and 0.25 <= g - ok[0][0] < 0.9 and G < FIN - 3:
        t, line, w = ok[0]
        for c in range(CHAIN):
            nx = [a for a in ap if a > G + 15]
            g2 = min(nx[0] + 30 if nx else FIN, FIN)
            ccut = G - 15
            ctag = '%s_ch%d' % (line, c)
            cpre = ctag + '_c.txt'
            open(cpre, 'w').writelines(open(line).readlines()[:ccut + 68])
            with ThreadPoolExecutor(CORES) as ex:
                rr = list(ex.map(lambda iv: (xds(run, cpre, g2, '%s_v%d' % (ctag, iv[0]), iv[1], FIN, ['celljump=1']), '%s_v%d' % (ctag, iv[0])), enumerate(VAR)))
            rr = sorted(r for r in rr if r[0] is not None)
            log('  chain %d from %d to %d: %s' % (c, ccut, g2, ' '.join('%.2f' % r[0] for r in rr) or 'NOGATE'))
            if not rr or g2 - rr[0][0] < 0.2:
                break
            if g2 - rr[0][0] >= 0.9 or g2 >= FIN - 3:
                if graft(rr[0][1], run, ccut + 3, g2, g2, FIN, 'kick %d (fired %d) forbidden over %d-%d, chained' % (E, F, E - w, E + w)):
                    return True
            line, G = rr[0][1], g2
    return False


T0, E0 = trace(BEST)
lo, hi = int(kw.get('from', 1020)), int(kw.get('to', 2540))
kicks = sorted(set((e, f) for e, f in E0 if lo <= e <= hi))
if 'kicks' in kw:
    want = set(int(x) for x in kw['kicks'].split(','))
    kicks = [k for k in kicks if k[0] in want]
log('kicktabu on %d: kicks %s, widths %s' % (finish(BEST), ' '.join('%d/%d' % k for k in kicks), WS))
done = set()
for e, f in kicks:
    if e in done:
        continue
    done.add(e)
    try:
        # the run may have changed (a gain): take the kick's explosion and fire tick from the current best
        _, Ec = trace(BEST)
        m = [k for k in Ec if abs(k[0] - e) <= 3]
        if not m:
            log('kick %d: gone from the current run' % e)
            continue
        try_kick(*min(m, key=lambda k: (abs(k[0] - e), k[1])))
    except Exception as ex:
        log('kick %d: error %r' % (e, ex))
log('done')
