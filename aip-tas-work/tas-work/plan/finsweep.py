#!/usr/bin/env python3
"""finsweep.py DIR CUTS [beam=8000] [cores=4] [vars=dschain_variants_rf_mix.txt] [extra='...']
x_ds from tas-work/kog_full_best.txt's own state at each cut straight to the finish (no gate, no graft: a faster line
is a full run), every variant; a gain is server-checked, committed and pushed. Writes DIR/log."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
cuts = [int(c) for c in sys.argv[2].split(',')]
kw = dict(a.split('=', 1) for a in sys.argv[3:] if '=' in a)
BEAM, CORES = kw.get('beam', '8000'), int(kw.get('cores', 4))
EXTRA = kw.get('extra', '').split()
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def finish(path):
    out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '2400'], capture_output=True, text=True).stdout
    f = [int(l.split()[2]) for l in out.splitlines() if l.startswith('T ') and 'FINISH' in l]
    return min(f) if f else None


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
FIN = finish(run)
L = open(run).readlines()
jobs = []
for c in cuts:
    pre = os.path.join(d, 'c%d.txt' % c)
    open(pre, 'w').writelines(L[:c + 68])
    for i, v in enumerate(VAR):
        jobs.append((c, i, pre, os.path.join(d, 'f%d_v%d.txt' % (c, i)), v))


def one(j):
    c, i, pre, out, v = j
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + pre, 'gate=finish', 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'out=' + out] + v.split() + EXTRA
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    f = finish(out) if os.path.exists(out) else None
    log('cut %d v%d: finish %s' % (c, i, f))
    return f, out, c


log('finsweep on %d: cuts %s beam %s extra %s' % (FIN, cuts, BEAM, EXTRA))
with ThreadPoolExecutor(CORES) as ex:
    res = [r for r in ex.map(one, jobs) if r[0] is not None]
res.sort()
if res and res[0][0] < FIN:
    f, out, c = res[0]
    chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), out], capture_output=True, text=True).stdout
    m = re.search(r'-> (\d+) ticks', chk)
    if m and int(m.group(1)) == f and 'frozen ticks 0' in chk and (finish(BEST) or 9999) > f:
        dst = os.path.join(TW, 'kog_full_%d.txt' % f)
        subprocess.run(['cp', out, dst])
        subprocess.run(['cp', out, BEST])
        with open(os.path.join(TW, 'NOTES.md'), 'a') as nf:
            nf.write('- finsweep cut %d (%s): **%d (%.2f s)**, server-checked.\n' % (c, os.path.relpath(out, TW), f, f / 50.0))
        msg = ('Full run %d (%.2f s): x_ds from rt %d straight to the finish\n\nServer-checked: TasReplay, no freeze, no double start.\n\n'
               'Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\nClaude-Session: https://claude.ai/code/session_01XxupAVqSvwb6MWzmsVwkue') % (f, f / 50.0, c)
        subprocess.run(['git', '-C', ROOT, 'add', os.path.relpath(dst, ROOT), 'tas-work/kog_full_best.txt', 'tas-work/NOTES.md'])
        subprocess.run(['git', '-C', ROOT, 'commit', '-q', '-m', msg])
        for k in range(4):
            if subprocess.run(['git', '-C', ROOT, 'push', '-u', 'origin', 'claude/sweet-maxwell-3t11yw']).returncode == 0:
                break
            time.sleep(2 ** (k + 1))
        log('NEW BEST %d (cut %d) committed and pushed' % (f, c))
    else:
        log('best %d (cut %d) failed the server check: %s' % (f, c, chk.strip().replace('\n', ' | ')))
log('done: best %s' % (res[0][:1] if res else None))
