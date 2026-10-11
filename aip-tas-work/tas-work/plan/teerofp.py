#!/usr/bin/env python3
"""teerofp.py DIR CUT END [tws=0.5,1] [vars=21,23] [beam=16000] [cores=4] [fpw=3] [fpg=25] [carry=1] [min=0.9]
Teero's shot structure over a stretch of tas-work/kog_full_best.txt: his shots (teero/shots_sim.tsv, video labels)
are mapped by place onto the run's race ticks (the run's tick at his firing position), and x_ds re-searches from the
run's state at CUT to the rt-END gate with that schedule as a fire plan (fireplan=..., fpw, fpg: shots only within fpw
of his, each window used - a lob may be placed by a retro shot up to fpg ticks later) while tracking his video track
lightly (tref ttrack 0.5 / 1). x_ds alone keeps the run's structure (the run's kicks look best at every step); plain
tracking without the plan spends his shots greedily and loses his structure. Found this way (Oct 11): the hop with
his lob onto the block over the right channel, 2.2 ahead of 2533 at rt 2284 by place.
The lines are measured against the run by place (the gate's progress measure can be off by many ticks in open
rooms); the best one a whole tick ahead (min) is carried to the finish by carry.py. Writes DIR/log."""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
CUT, END = int(sys.argv[2]), int(sys.argv[3])
kw = dict(a.split('=', 1) for a in sys.argv[4:] if '=' in a)
TWS = kw.get('tws', '0.5,1').split(',')
VS = kw.get('vars', '21,23').split(',')
BEAM, CORES = kw.get('beam', '16000'), int(kw.get('cores', 4))
FPW, FPG = kw.get('fpw', '3'), kw.get('fpg', '25')
MIN = float(kw.get('min', 0.9))
VAR = {}
for l in open(os.path.join(HERE, 'dschain_variants_rf_mix2.txt')):
    if l.startswith('seed='):
        VAR[l.split()[0][5:]] = l.strip() + (' incjump=1' if l.startswith('seed=21') else '') + ' cellreload=1'
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm):
    A = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            A[int(a[2])] = (float(a[3]), float(a[4]), 'DEAD' in a)
    return A


def match(A, B, t0, back=40):
    """per tick t of A from t0: B's (fractional) tick at A's position (monotone projection onto B's path), offset"""
    out, m = {}, None
    for t in sorted(A):
        if t < t0 or A[t][2]:
            continue
        p, best = A[t], None
        for r in (range(t - back, t + back) if m is None else range(int(m) - 8, int(m) + 14)):
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
        out[t] = (m, best[0])
    return out


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
R = trace(run, 960)
T = {}
for l in open(os.path.join(TW, 'teero_track.txt')):
    a = l.split()
    T[int(a[0])] = (float(a[1]), float(a[2]), False)
# our tick -> his label at the same place; inverted: his label -> our tick
M = match(R, T, 990, back=60)
inv = sorted((m, t) for t, (m, off) in M.items() if off < 40)


def ours(label):
    for (m0, t0), (m1, t1) in zip(inv, inv[1:]):
        if m0 <= label <= m1 and m1 > m0:
            return t0 + (label - m0) / (m1 - m0) * (t1 - t0)
    return None


toff = CUT - M[CUT][0]
shots = []
for l in open(os.path.join(TW, 'teero', 'shots_sim.tsv')):
    a = l.split('\t')
    if a[0] == 'n':
        continue
    o = ours(float(a[1]))
    if o is not None and CUT + 2 <= o <= END + 20:
        shots.append(int(round(o)))
plan = ','.join(str(s) for s in shots)
log('teerofp %d -> %d on %d: toff %.2f, his shots by place at our %s' % (CUT, END, int(sorted(t for t in R)[-1]), toff, plan or '-'))
if kw.get('dry', '0') == '1':
    sys.exit(0)
pre = os.path.join(d, 'c%d.txt' % CUT)
open(pre, 'w').writelines(open(run).readlines()[:CUT + 68])


def xds(out, var, extra):
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + pre, 'gate=rt%d' % END, 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'gatelat=64', 'out=' + out] + var.split() + extra
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    return os.path.exists(out) and 'GATE t' in open(out + '.log').read()


jobs, BEST_EX = [], {}
for tw in TWS:
    for v in VS:
        out = os.path.join(d, 'l%d_%d_t%s_v%s.txt' % (CUT, END, tw, v))
        # tw 'pF': position-only reference (tpath, beam share F) where his timing is out of reach (time tracking with
        # the fixed offset killed every lineage at the shaft entry, where he is 5-8 px/t faster)
        ex = (['tpath=' + os.path.join(TW, 'teero_track.txt'), 'tpfrac=' + tw[1:]] if tw.startswith('p') else
              ['tref=' + os.path.join(TW, 'teero_track.txt'), 'ttrack=' + tw, 'toff=%.2f' % toff])
        if plan:
            ex += ['fireplan=' + plan, 'fpw=' + FPW, 'fpg=' + FPG]
        jobs.append((out, VAR[v], ex))
        BEST_EX[out] = [a for a in ex if not a.startswith(('fpw', 'fpg'))]
with ThreadPoolExecutor(CORES) as ex:
    ok = list(ex.map(lambda j: (xds(*j), j[0]), jobs))
best = None
for good, out in ok:
    if not good:
        log('  %s: NOGATE' % os.path.basename(out))
        continue
    L = match(trace(out, CUT - 40), R, CUT + 2, back=20)
    # lead = run's tick at the line's place - line's tick (positive: the line is ahead)
    on = [(m - t, t) for t, (m, off) in L.items() if off < 12 and t >= END - 25]
    if not on:
        log('  %s: off the run\'s path at the end' % os.path.basename(out))
        continue
    lead, t = on[-1]
    log('  %s: lead %.2f at %d (max %.2f)' % (os.path.basename(out), lead, t, max(on)[0]))
    if best is None or lead > best[0]:
        best = (lead, t, out)
if best and best[0] >= MIN and kw.get('carry', '1') == '1':
    cd = best[2][:-4] + '_carry'
    log('carry %s from %d (lead %.2f)' % (os.path.basename(best[2]), best[1] - 4, best[0]))
    # stage 0 of the carry keeps the line's own light tracking (and the fire plan's later windows)
    x0 = [a for a in BEST_EX.get(best[2], []) if not a.startswith('fireplan')] + (['fireplan=' + plan, 'fpw=' + FPW, 'fpg=' + FPG] if plan else [])
    subprocess.run(['python3', os.path.join(HERE, 'carry.py'), cd, best[2], str(best[1] - 4), 'cores=%d' % CORES, 'stages=6',
                    'x0=' + ';'.join(x0), 'what=Teero structure %d-%d (fire plan + light tracking), carried' % (CUT, END)], capture_output=True)
    cl = open(os.path.join(cd, 'log')).read() if os.path.exists(os.path.join(cd, 'log')) else ''
    log('  carry: %s' % ' | '.join(l[9:] for l in cl.strip().splitlines()[-3:]))
log('done')
