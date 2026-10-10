#!/usr/bin/env python3
"""carry.py DIR LINE CUT [stages=6] [beam=30000] [cores=4] [vars=dschain_variants_rf_mix2.txt] [what=...]
Carries a line's lead over tas-work/kog_full_best.txt to the finish: big-beam x_ds stages (incumbent = the run) from
LINE cut at CUT to the gate after the next turn (apex + 30), then on from each stage's best line (cut 15 before its
gate), until a stage's line has rejoined the run's own states some whole ticks early - found by splicing it onto the
run's inputs shifted by D = 1..3 ticks at every tick of the stage and replaying (no graft search) - or its lead is
gone. The x_ds re-search after a change needs the big beam: the S-bend line (Oct 10) kept its lead through the corner
and the hop at 20000 / 30000 and lost it at 4000-12000, and its 30000 stage was exactly the run one tick early from
rt 2247 (2541 -> 2540). A faster splice is server-checked, committed and pushed. Writes DIR/log."""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
LINE, CUT = os.path.abspath(sys.argv[2]), int(sys.argv[3])
kw = dict(a.split('=', 1) for a in sys.argv[4:] if '=' in a)
# commits go to the checked-out branch, with this session's link (session=URL overrides)
BRANCH = subprocess.run(['git', '-C', ROOT, 'rev-parse', '--abbrev-ref', 'HEAD'], capture_output=True, text=True).stdout.strip()
SESSION = kw.get('session', 'https://claude.ai/code/session_01FMtZNoArAZJ4aqsFiXN4kh')
STAGES, BEAM, CORES = int(kw.get('stages', 6)), kw.get('beam', '30000'), int(kw.get('cores', 4))
WHAT = kw.get('what', os.path.basename(LINE))
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix2.txt'))) if l.strip() and not l.startswith('#')]
# the two variants that carried the S-bend line (seed 21 with incjump, seed 23), unless all are asked for
if kw.get('allvars', '0') != '1':
    VAR = [v + (' incjump=1' if v.startswith('seed=21') else '') + ' cellreload=1' for v in VAR if v.startswith(('seed=21', 'seed=23'))]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm):
    T = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(p=(float(a[3]), float(a[4])), v=(float(a[5]), float(a[6])), s=float(a[7]), fin='FINISH' in a, dead='DEAD' in a)
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


def xds(prefix, gate, out, var):
    g = 'gate=finish' if gate >= FIN - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'out=' + out] + var.split()
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def splices(line, lo, hi):
    """splice LINE[:k] + RUN[k + D:] for k in lo..hi, D = 3, 2, 1: the fastest finishing one"""
    a, b = open(line).readlines(), open(run).readlines()
    best = None
    for D in (3, 2, 1):
        for k in range(lo, min(hi, len(a) - 68) + 1):
            p = os.path.join(d, 'sp.txt')
            open(p, 'w').writelines(a[:k + 68] + b[k + 68 + D:])
            f = finish(p)
            if f is not None and f < FIN and (best is None or f < best[0]):
                best = (f, k, D)
                subprocess.run(['cp', p, os.path.join(d, 'sp_%d_k%d_D%d.txt' % (f, k, D))])
        if best:
            break
    return best


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
        nf.write('- carry %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): carry %s (big-beam x_ds stages from a leading line, exact splice)\n\n'
           'Server-checked: TasReplay, no freeze, no double start.\n\nCo-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\n'
           'Claude-Session: ' + SESSION) % (ticks, ticks / 50.0, what)
    subprocess.run(['git', '-C', ROOT, 'add', os.path.relpath(dst, ROOT), 'tas-work/kog_full_best.txt', 'tas-work/NOTES.md'])
    subprocess.run(['git', '-C', ROOT, 'commit', '-q', '-m', msg])
    for k in range(4):
        if subprocess.run(['git', '-C', ROOT, 'push', '-u', 'origin', BRANCH]).returncode == 0:
            break
        time.sleep(2 ** (k + 1))
    log('  NEW BEST %d committed and pushed' % ticks)
    return True


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
FIN = finish(run)
ap = apexes(trace(run, 990))
log('carry %s from %d on %d (beam %s, %d variants)' % (WHAT, CUT, FIN, BEAM, len(VAR)))
line, cut = LINE, CUT
for st in range(STAGES):
    nx = [a for a in ap if a > cut + 25]
    g = min(nx[0] + 30 if nx else FIN, FIN)
    pre = os.path.join(d, 's%d_c%d.txt' % (st, cut))
    open(pre, 'w').writelines(open(line).readlines()[:cut + 68])
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda iv: (xds(pre, g, os.path.join(d, 's%d_v%d.txt' % (st, iv[0])), iv[1]), os.path.join(d, 's%d_v%d.txt' % (st, iv[0]))), enumerate(VAR)))
    ok = sorted(r for r in res if r[0] is not None)
    log('stage %d from %d to %d: %s' % (st, cut, g, ' '.join('%.2f' % r[0] for r in ok) or 'NOGATE'))
    if not ok:
        break
    for t, l in ok:
        if t < g - 0.5 or g >= FIN - 3:
            if g >= FIN - 3:
                f = finish(l)
                if f is not None and f < FIN:
                    log('  finishes %d' % f)
                    if publish(l, f, WHAT):
                        sys.exit(0)
            b = splices(l, cut + 1, g)
            if b:
                log('  splice %s: finish %d at k %d D %d' % (os.path.basename(l), b[0], b[1], b[2]))
                if publish(os.path.join(d, 'sp_%d_k%d_D%d.txt' % b), b[0], '%s (stage %d, splice at %d, %d ticks early)' % (WHAT, st, b[1], b[2])):
                    sys.exit(0)
    t, l = ok[0]
    if g - t < 0.2 or g >= FIN - 3:
        log('lead gone (%.2f)' % (g - t))
        break
    line, cut = l, g - 15
log('done')
