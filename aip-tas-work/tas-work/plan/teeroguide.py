#!/usr/bin/env python3
"""teeroguide.py DIR CUT TOFF LABEL [gate2=finish|RT|auto] [cores=4] [beam=8000] [vars=dschain_variants_rf_mix.txt]
   teeroguide.py DIR sweep [from=1000] [to=2540] [min=1.0]: every stretch where Teero gains >= min on the current best
Teero-guided re-search of one stretch of tas-work/kog_full_best.txt where he is faster (analysis/lagat.py): x_ds from
the run's state at CUT tracking his video track in time (tref=teero_track.txt ttrack 0.5 / 1, toff = TOFF = our race
tick - his label there) to his label LABEL (the end of the stretch), every variant; each tracked line is then
measured against the run by place (lead, path offset), cut 3 ticks before its largest on-line lead and re-searched
untracked (gateres=5,0.3) to gate2 (the finish, an RT, or auto: the run's next turn apex + 30); every untracked line is
spliced onto the run's inputs shifted by D = 3, 2, 1 ticks at every tick of the stage and replayed (the untracked
re-search tends to rejoin the run's own states exactly one tick early: 2541 -> 2540 -> 2539).
Found this way: the S-bend top (rt 1945-1970), 3.7 ahead of 2541 at 1965 with tracking, which x_ds alone never keeps
(it is behind with less energy until the top). A finishing gain is server-checked, committed and pushed. Writes DIR/log."""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
SWEEP = sys.argv[2] == 'sweep'
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
CORES, BEAM, G2 = int(kw.get('cores', 4)), kw.get('beam', '8000'), kw.get('gate2', 'auto')
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_mix.txt'))) if l.strip() and not l.startswith('#')]
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
            T[int(a[2])] = (float(a[3]), float(a[4]), 'FINISH' in a, 'DEAD' in a, float(a[7]), (float(a[5]), float(a[6])))
    return T


def finish(path):
    T = trace(path, 2400)
    f = [t for t in T if T[t][2]]
    return min(f) if f else None


def leads(line, run, r0):
    """per tick of LINE from r0: (lead in ticks over RUN at the same place, distance to RUN's path)"""
    A, B = trace(line, r0 - 40), trace(run, r0 - 40)
    out, m = {}, None
    for t in sorted(A):
        if t < r0 or A[t][3]:
            continue
        p, best = A[t], None
        for r in (range(t - 30, t + 30) if m is None else range(int(m) - 6, int(m) + 12)):
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
        out[t] = (m - t, best[0])
    return out


def xds(prefix, gate, out, var, extra):
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, gate, 'incforce=0', 'threads=1', 'beam=' + BEAM,
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
        nf.write('- teeroguide %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): teeroguide %s (x_ds tracking Teero over the stretch, untracked re-search)\n\n'
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


def apexes(T):
    ts = sorted(t for t in T if t > 1000)
    out = []
    for t in ts:
        w = [T[k][4] for k in range(t - 15, t + 16) if k in T]
        if T[t][4] == min(w) and T[t][4] < 0.85 * max(w) and (not out or t - out[-1] > 20) and t - 20 in T and t + 20 in T:
            (ax, ay), (bx, by) = T[t - 20][5], T[t + 20][5]
            if (ax * bx + ay * by) / ((math.hypot(ax, ay) * math.hypot(bx, by)) or 1) < 0.5:
                out.append(t)
    return out


def splices(line, lo, hi):
    a, b = open(line).readlines(), open(run).readlines()
    best = None
    for D in (3, 2, 1):
        for k in range(lo, min(hi, len(a) - 68) + 1):
            p = os.path.join(d, 'sp.txt')
            open(p, 'w').writelines(a[:k + 68] + b[k + 68 + D:])
            f = finish(p)
            if f is not None and f < FIN and (best is None or f < best[0]):
                best = (f, k, D, os.path.join(d, 'sp_%d_%s_k%d_D%d.txt' % (f, os.path.basename(line)[:40], k, D)))
                subprocess.run(['cp', p, best[3]])
        if best:
            break
    return best


def guide(CUT, TOFF, LABEL):
    global FIN
    tag = os.path.join(d, 'g%d' % CUT)
    pre = tag + '_p.txt'
    open(pre, 'w').writelines(open(run).readlines()[:CUT + 68])
    jobs = [('%s_t%s_v%d.txt' % (tag, tw, i), v, ['tref=' + os.path.join(TW, 'teero_track.txt'), 'ttrack=' + tw, 'toff=%g' % TOFF])
            for tw in ('1', '0.5') for i, v in enumerate(VAR[:3])]
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda j: (xds(pre, 'gate=rt%d' % LABEL, j[0], j[1], j[2]), j[0]), jobs))
    log('cut %d toff %g to his label %d (run %d): tracked %d of %d' % (CUT, TOFF, LABEL, FIN, sum(r[0] is not None for r in res), len(res)))
    cands = []
    for t, line in res:
        if t is None:
            continue
        L = leads(line, run, CUT + 2)
        on = [(L[k][0], k) for k in L if L[k][1] < 8]
        if not on:
            continue
        lead, k = max(on)
        log('  %s: max on-line lead %.2f at %d (end %d: %.2f)' % (os.path.basename(line), lead, k, max(L), L[max(L)][0]))
        if lead >= 0.5:
            cands.append((lead, k, line))
    cands.sort(reverse=True)
    best = None
    for lead, k, line in cands[:3]:
        for c2 in sorted({max(CUT + 2, k - 3), max(CUT + 2, k - 10)}):
            if G2 == 'finish':
                g, gt = 'gate=finish', FIN
            elif G2 == 'auto':
                nx = [a for a in AP if a > c2 + 25]
                gt = min(nx[0] + 30 if nx else FIN, FIN)
                g = 'gate=finish' if gt >= FIN - 3 else 'gate=rt%d' % gt
            else:
                g, gt = 'gate=rt%s' % G2, int(G2)
            p2 = line[:-4] + '_c%d.txt' % c2
            open(p2, 'w').writelines(open(line).readlines()[:c2 + 68])
            with ThreadPoolExecutor(CORES) as ex:
                rr = list(ex.map(lambda iv: (xds(p2, g, '%s_u%d.txt' % (p2[:-4], iv[0]), iv[1], ['gateres=5,0.3']), '%s_u%d.txt' % (p2[:-4], iv[0])), enumerate(VAR)))
            rr = sorted(r for r in rr if r[0] is not None)
            log('  untracked from %d (lead %.2f) to %d: %s' % (c2, lead, gt, ' '.join('%.2f' % r[0] for r in rr) or 'NOGATE'))
            for t2, l2 in rr:
                f = finish(l2) if gt >= FIN - 3 else None
                if f is not None and f < FIN and (best is None or f < best[0]):
                    best = (f, 0, 0, l2)
                if t2 < gt - 0.5:
                    b = splices(l2, c2 + 1, gt)
                    if b:
                        log('    splice %s at %d, %d early: %d' % (os.path.basename(l2), b[1], b[2], b[0]))
                        if best is None or b[0] < best[0]:
                            best = b
            if best:
                break
        if best:
            break
    if best and publish(best[3], best[0], 'cut %d (Teero tracked to his label %d)' % (CUT, LABEL)):
        log('NEW BEST %d' % best[0])
        subprocess.run(['cp', BEST, run])
        FIN = best[0]
        return True
    return False


def stretches(lo, hi, mn):
    """(start, end, gain, lag at start, his label at end) where Teero gains >= mn on the run, from matched points"""
    out = subprocess.run(['python3', os.path.join(TW, 'analysis', 'lagat.py'), run, str(lo), str(hi), '5'], capture_output=True, text=True).stdout
    G = []
    for l in out.splitlines():
        a = l.split()
        if len(a) >= 7 and float(a[6]) < 12:
            G.append((int(a[0]), float(a[2])))
    segs, i = [], 0
    while i < len(G) - 1:
        j = i
        while j + 1 < len(G) and G[j + 1][1] >= G[j][1] - 0.15:
            j += 1
        if G[j][1] - G[i][1] >= mn:
            segs.append((G[i][0], G[j][0], G[j][1] - G[i][1], G[i][1], int(round(G[j][0] - G[j][1])) + 3))
        i = j + 1
    return segs


run = os.path.join(d, 'run.txt')
subprocess.run(['cp', BEST, run])
FIN = finish(run)
AP = apexes(trace(run, 990))
if not SWEEP:
    guide(int(sys.argv[2]), float(sys.argv[3]), int(sys.argv[4]))
else:
    segs = stretches(int(kw.get('from', 1000)), int(kw.get('to', 2540)), float(kw.get('min', 1.0)))
    log('sweep on %d: stretches %s' % (FIN, ' '.join('%d-%d(+%.1f)' % s[:3] for s in segs)))
    for s0, s1, gain, lag0, lab in sorted(segs, key=lambda s: -s[2]):
        try:
            guide(s0 - 5, round(lag0 * 2) / 2, lab)
            AP = apexes(trace(run, 990))
        except Exception as ex:
            log('stretch %d-%d: error %r' % (s0, s1, ex))
log('done')
