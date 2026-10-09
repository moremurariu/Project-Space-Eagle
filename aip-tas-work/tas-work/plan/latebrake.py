#!/usr/bin/env python3
"""latebrake.py DIR [turns=A1,..] [vmin=40] [cores=4] [vars=dschain_variants_rf_mix.txt] [allep=0] [ks=2,4] [full=1]
Teero brakes later than we do before turns and carries 7-14 px/t more on those approaches (sp/teerov.py). For every
turn A (speed minimum with a >= 60 deg heading change) of tas-work/kog_full_best.txt whose approach has a
direction-key braking episode (dir 0 or against vx at |vx| > 8 in the air, |v| >= vmin, starting E0 in A-60..A-5):
the direction is held along vx from E0 for k ticks (k = the whole episode, half of it), then x_ds re-searches from
E0 + k to the gate after the turn (the next turn + 30) with celljump, once with jumps forbidden until A - 3 (the air
jump kept for the turn: 2551 -> 2550 at the 1800 U-turn) and once without; the best line is spliced onto the run where
it is the run's exact state some ticks earlier (multigraft), and a server-checked gain is committed and pushed."""
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
CORES, VMIN = int(kw.get('cores', 4)), float(kw.get('vmin', 40))
ALLEP = int(kw.get('allep', 0))  # every braking episode of the approach, not only the first
KS = [int(x) for x in kw.get('ks', '0').split(',') if int(x) > 0]  # hold lengths (besides the full / half episode)
FULL = int(kw.get('full', 1))
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm=990):
    T = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(v=(float(a[5]), float(a[6])), s=float(a[7]), dir=int(a[14]), fin='FINISH' in a)
    return T


def finish(path):
    T = trace(path, 2400)
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


def episodes(T, lo, hi):
    """direction-key braking episodes starting in lo..hi: (start, end) of consecutive ticks whose dir input brakes vx"""
    out, cur = [], None
    for t in range(lo, hi + 30):
        if t - 1 not in T or t not in T:
            continue
        vx = T[t - 1]['v'][0]
        bad = abs(vx) > 8 and (T[t]['dir'] == 0 or T[t]['dir'] * vx < 0) and T[t - 1]['s'] >= VMIN
        if bad and cur and t <= cur[1] + 4:  # gaps of up to 3 ticks belong to the same braking
            cur[1] = t
        elif bad and t <= hi:
            cur = [t, t]
            out.append(cur)
    return [tuple(e) for e in out]


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
        nf.write('- latebrake %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): latebrake %s (braking delayed before a turn, x_ds re-search + splice)\n\n'
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


def try_turn(A):
    run = os.path.join(d, 'run_%d.txt' % A)
    subprocess.run(['cp', BEST, run])
    FIN = finish(run)
    T = trace(run)
    ap = apexes(T)
    nx = [a for a in ap if a > A + 25]
    G = min((nx[0] + 30) if nx else FIN, FIN)
    eps = episodes(T, A - 60, A - 5)
    if not eps:
        log('turn %d: no braking episode at |v| >= %.0f' % (A, VMIN))
        return False
    L = open(run).readlines()
    jobs = []
    for E0, E1 in (eps if ALLEP else eps[:1]):
        jobs += turn_jobs(A, E0, E1, L, T, run)
    log('turn %d (run %d): episodes %s, %d searches, gate %d' % (A, FIN, eps, len(jobs), G))
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda j: (xds(run, j[0], G, j[1], j[2], FIN, j[3]), j[1], j[4], j[5], j[6]), jobs))
    ok = sorted(r for r in res if r[0] is not None)
    log('  gate %d: %s' % (min(G, FIN), ' '.join('%.2f(e%d k%d%s)' % (r[0], r[4], r[2], 'nj' if r[3] else '') for r in ok[:8]) or 'NOGATE'))
    for t, line, k, nj, E0 in ok[:3]:
        if t > min(G, FIN) - 0.9:
            break
        gd = line + '_mg'
        mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), line, run, gd, str(E0 + k + 3), str(min(G, FIN - 5)), '6', '3', str(CORES)],
                            capture_output=True, text=True).stdout
        log('  multigraft %s: %s' % (os.path.basename(line), ' | '.join(mg.strip().splitlines()[-2:])))
        cs = [os.path.join(gd, g) for g in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
        if G >= FIN:
            cs.append(line)
        best = None
        for p in cs:
            f = finish(p)
            if f is not None and (best is None or f < best[0]):
                best = (f, p)
        if best and best[0] < FIN and publish(best[1], best[0], 'turn %d (dir held %d-%d%s)' % (A, E0, E0 + k - 1, ', jump kept' if nj else '')):
            return True
    return False


def turn_jobs(A, E0, E1, L, T, run):
    jobs = []
    n = E1 - E0 + 1
    ks = set(min(n, x) for x in KS) if KS else {max(2, n // 2)}
    for k in sorted(ks | ({n} if FULL else set()), reverse=True):
        pre = os.path.join(d, 't%d_e%d_k%d_p.txt' % (A, E0, k))
        out = L[:E0 + k - 1 + 68]
        for rt in range(E0, E0 + k):
            a = out[rt + 68 - 1].split()
            vx = T[rt - 1]['v'][0]
            a[0] = '1' if vx > 0 else '-1'
            out[rt + 68 - 1] = ' '.join(a) + '\n'
        open(pre, 'w').writelines(out)
        for nj in (True, False):
            extra = ['celljump=1'] + (['nojump=%d,%d' % (E0 + k, A - 3)] if nj else [])
            for i, v in enumerate(VAR):
                jobs.append((pre, '%s/t%d_e%d_k%d_%s_v%d.txt' % (d, A, E0, k, 'nj' if nj else 'j', i), v, extra, k, nj, E0))
    return jobs


T0 = trace(BEST)
turns = [int(x) for x in kw['turns'].split(',')] if 'turns' in kw else apexes(T0)
log('latebrake on %d: turns %s' % (finish(BEST), turns))
for A in turns:
    try:
        try_turn(A)
    except Exception as ex:
        log('turn %d: error %r' % (A, ex))
log('done')
