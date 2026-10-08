#!/usr/bin/env python3
"""structsweep.py RUN DIR [shots=F1,F2,... | from=RT to=RT] [cores=2] [pre=20] [g1=60] [c2=30] [g2=140] [beam=4000]
[variants=FILE] [mode=drop|late|early]

Shot-structure sweep (the recipe that gave 2580 -> 2576 at U-turn 1, NOTES "Shot structure from Teero"): for every
shot of RUN (race tick F of its fire step), x_ds re-plans the run from F - pre with that shot forbidden around its own
time (nokick=F-2,F+4 nokickall=1: the gun is kept for later, so retro lobs / exit stacks become possible), to a gate
g1 ticks after F (stage 1, every variant); the best stage-1 line is re-searched from F + c2 (after the change, so the
following kicks can be re-timed) to F + g2 (stage 2); a stage-2 line ahead of RUN by >= 1 tick is grafted back onto
RUN (multigraft.py, every on-line cut, largest D first) and a finishing graft that passes the server check is the new
run (DIR/best_<ticks>.txt, and RUN is replaced). mode=late / early: nokick=F-2,F+8 / F-8,F+2 instead.
Writes DIR/sweep.log."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
run, d = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
kw = dict(a.split('=', 1) for a in sys.argv[3:] if '=' in a)
CORES = int(kw.get('cores', 2))
PRE, G1, C2, G2 = int(kw.get('pre', 20)), int(kw.get('g1', 60)), int(kw.get('c2', 30)), int(kw.get('g2', 140))
BEAM = kw.get('beam', '4000')
MODE = kw.get('mode', 'drop')
VAR = [l.strip() for l in open(kw.get('variants', os.path.join(HERE, 'dschain_variants_rf.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'sweep.log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm):
    T = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(fire=int(a[17]), rl=int(a[21]), fin='FINISH' in a)
    return T


def finish(path):
    T = trace(path, 2400)
    f = [t for t in T if T[t]['fin']]
    return min(f) if f else None


def shots(path):
    T = trace(path, 990)
    return [t for t in sorted(T) if T[t]['fire'] and t - 1 in T and T[t - 1]['rl'] == 0 and T[t]['rl'] > 0 and t > 1000]


def head(path, rt, out):
    with open(path) as f:
        lines = f.readlines()
    with open(out, 'w') as g:
        g.writelines(lines[:rt + 68])


def xds(prefix, gate, out, var, extra=()):
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, 'gate=rt%d' % gate, 'incforce=0', 'threads=1',
           'beam=' + BEAM, 'verbose=0', 'out=' + out] + var.split() + list(extra)
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def stage(prefix, gate, tag, extra=()):
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda iv: (xds(prefix, gate, '%s_v%d' % (tag, iv[0]), iv[1], extra), '%s_v%d' % (tag, iv[0])), enumerate(VAR)))
    ok = [r for r in res if r[0] is not None]
    return min(ok) if ok else (None, None)


def try_shot(F):
    tag = os.path.join(d, 's%d_%s' % (F, MODE))
    lo, hi = {'drop': (F - 2, F + 4), 'late': (F - 2, F + 8), 'early': (F - 8, F + 2)}[MODE]
    cut = F - PRE
    head(run, cut, tag + '_p.txt')
    t1, l1 = stage(tag + '_p.txt', F + G1, tag + '_a', ('nokick=%d,%d' % (lo, hi), 'nokickall=1'))
    log('shot %d: stage 1 (cut %d, gate %d): %s' % (F, cut, F + G1, 'NOGATE' if t1 is None else '%.2f (%+.2f)' % (t1, F + G1 - t1)))
    if t1 is None or t1 > F + G1 + 3:
        return
    head(l1, F + C2, tag + '_c.txt')
    t2, l2 = stage(tag + '_c.txt', F + G2, tag + '_b')
    log('shot %d: stage 2 (cut %d, gate %d): %s' % (F, F + C2, F + G2, 'NOGATE' if t2 is None else '%.2f (%+.2f)' % (t2, F + G2 - t2)))
    if t2 is None or t2 > F + G2 - 1:
        return
    gd = tag + '_mg'
    out = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(F + C2 + 5), str(F + G2), '6', '4', str(CORES)],
                         capture_output=True, text=True).stdout
    log('shot %d: multigraft: %s' % (F, ' | '.join(out.strip().splitlines()[-3:])))
    best = None
    for g in sorted(os.listdir(gd)) if os.path.isdir(gd) else []:
        p = os.path.join(gd, g)
        f = finish(p)
        if f is not None and (best is None or f < best[0]):
            best = (f, p)
    cur = finish(run)
    if best and best[0] < cur:
        chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), best[1]], capture_output=True, text=True).stdout
        m = re.search(r'-> (\d+) ticks', chk)
        if m and int(m.group(1)) == best[0] and 'frozen ticks 0' in chk:
            dst = os.path.join(d, 'best_%d.txt' % best[0])
            subprocess.run(['cp', best[1], dst])
            subprocess.run(['cp', best[1], run])
            log('shot %d: NEW BEST %d (server-checked) from %s' % (F, best[0], best[1]))
        else:
            log('shot %d: graft %d failed the server check: %s' % (F, best[0], chk.strip().replace('\n', ' | ')))


if 'shots' in kw:
    todo = [int(x) for x in kw['shots'].split(',')]
else:
    todo = [s for s in shots(run) if int(kw.get('from', 1000)) <= s <= int(kw.get('to', 2600))]
log('structsweep on %s (finish %s), mode %s, shots %s' % (run, finish(run), MODE, todo))
for F in todo:
    # shot ticks move when the run changes (a graft ahead): re-map to the nearest current shot
    cur = shots(run)
    near = min(cur, key=lambda s: abs(s - F)) if cur else F
    if abs(near - F) <= 6:
        F = near
    try_shot(F)
log('done')
