#!/usr/bin/env python3
"""rejoin.py DIR [from=1000] [to=2480] [step=25] [len=80] [beam=30000] [cores=4] [vars=dschain_variants_rf_mix2.txt]
Big-beam x_ds from tas-work/kog_full_best.txt's own state at every cut (from..to by step) to cut + len (incumbent =
the run, the two variants that carried the S-bend line); a line a whole tick ahead at its gate is spliced onto the
run's inputs shifted by D = 3, 2, 1 ticks at every tick of the window and replayed: an x_ds line that gets a tick
ahead tends to converge onto the run's own states exactly (Oct 10: 2541 -> 2540 -> 2539). A faster splice is
server-checked, committed and pushed, and the following cuts use the new run. Writes DIR/log."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
LO, HI, STEP, LEN = int(kw.get('from', 1000)), int(kw.get('to', 2480)), int(kw.get('step', 25)), int(kw.get('len', 80))
BEAM, CORES = kw.get('beam', '30000'), int(kw.get('cores', 4))
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix2.txt'))) if l.strip() and not l.startswith('#')]
VAR = [v + (' incjump=1' if v.startswith('seed=21') else '') + ' cellreload=1' for v in VAR if v.startswith(('seed=21', 'seed=23'))]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def finish(path):
    out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '2400'], capture_output=True, text=True).stdout
    f = [int(l.split()[2]) for l in out.splitlines() if l.startswith('T ') and 'FINISH' in l]
    return min(f) if f else None


def xds(prefix, gate, out, var):
    g = 'gate=finish' if gate >= FIN - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'out=' + out] + var.split()
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def splices(line, lo, hi):
    a, b = open(line).readlines(), open(run).readlines()
    best = None
    for D in (3, 2, 1):
        for k in range(lo, min(hi, len(a) - 68) + 1):
            p = os.path.join(d, 'sp.txt')
            open(p, 'w').writelines(a[:k + 68] + b[k + 68 + D:])
            f = finish(p)
            if f is not None and f < FIN and (best is None or f < best[0]):
                best = (f, k, D, line + '_sp_k%d_D%d.txt' % (k, D))
                subprocess.run(['cp', p, best[3]])
        if best:
            return best
    return None


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
        nf.write('- rejoin %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): rejoin %s (big-beam x_ds window, exact splice)\n\n'
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


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
FIN = finish(run)
log('rejoin on %d: cuts %d..%d step %d, windows %d, beam %s' % (FIN, LO, HI, STEP, LEN, BEAM))
cuts = list(range(LO, HI + 1, STEP))
# two cuts at a time (one per variant pair) keep the 4 cores busy
for i in range(0, len(cuts), 2):
    batch = cuts[i:i + 2]
    jobs = []
    for c in batch:
        pre = os.path.join(d, 'c%d.txt' % c)
        open(pre, 'w').writelines(open(run).readlines()[:c + 68])
        for k, v in enumerate(VAR):
            jobs.append((c, pre, os.path.join(d, 'w%d_v%d.txt' % (c, k)), v))
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda j: (j[0], xds(j[1], min(j[0] + LEN, FIN), j[2], j[3]), j[2]), jobs))
    for c, t, out in res:
        g = min(c + LEN, FIN)
        log('cut %d -> %d: %s' % (c, g, '%.2f' % t if t is not None else 'NOGATE'))
        if t is None or t > g - 0.9:
            continue
        b = splices(out, c + 1, g)
        if b:
            log('  splice %s at %d, %d early: %d' % (os.path.basename(out), b[1], b[2], b[0]))
            if publish(b[3], b[0], 'cut %d (spliced at %d, %d early)' % (c, b[1], b[2])):
                subprocess.run(['cp', BEST, run])
                FIN = b[0]
log('done')
