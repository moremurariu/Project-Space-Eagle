#!/usr/bin/env python3
"""structsearch.py DIR [turns=A1,A2,...] [k=3] [beam=4000] [cores=4] [seed=1]
Structure layer over x_ds: for every turn of tas-work/kog_full_best.txt (speed minimum A) whose approach has a shot
(fired F0, exploding in A-40..A-2), the alternative plan Teero uses at U-turn 1 is generated and evaluated, and kept
when it is faster (nothing is forbidden by hand: our plan and the alternative compete on the same long-horizon test).
  1. base: x_ds from F0-12 to A+5 without that shot (nokick around F0, cellreload) -> a line that saves the grenade
  2. pre-fires: x_pfscan on the base line, fired A-32..A-8 (>= 25 after the previous shot), exploding A-3..A+20,
     scored along the run's exit velocity -> top k (fire tick, aim)
  3. per pre-fire: forced prefix -> x_ds to A+55 (stage 1; cellreload, shlate) -> x_opt polish F0-10..A+40 (ref =
     the run) -> x_ds from A+30 to the next turn + 30 (stage 2) -> multigraft onto the run -> server check
A gain becomes kog_full_<n>.txt / kog_full_best.txt and is committed and pushed. Writes DIR/log."""
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
K, BEAM, CORES, SEED = int(kw.get('k', 3)), kw.get('beam', '4000'), int(kw.get('cores', 4)), int(kw.get('seed', 1))
VAR = [l.strip() for l in open(os.path.join(HERE, 'dschain_variants_rf.txt')) if l.strip() and not l.startswith('#')]
NEW = ['cellreload=1', 'shlate=12']
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def trace(path, frm=990):
    T, E = {}, {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(frm)], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 21 and a[0] == 'T':
            T[int(a[2])] = dict(p=(float(a[3]), float(a[4])), v=(float(a[5]), float(a[6])), s=float(a[7]), fire=int(a[17]),
                                rl=int(a[21]), fin='FINISH' in a)
        elif a and a[0] == 'E':
            E.setdefault(int(a[2]), []).append(1)
    return T, E


def finish(path):
    T, _ = trace(path, 2400)
    f = [t for t in T if T[t]['fin']]
    return min(f) if f else None


def head(path, rt, out, extra=None):
    with open(path) as f:
        lines = f.readlines()
    with open(out, 'w') as g:
        g.writelines(lines[:rt + 68])
        if extra:
            g.write(extra + '\n')


def line_at(path, rt):
    with open(path) as f:
        return f.readlines()[rt + 68 - 1].split()


def xds(run, prefix, gate, out, var, extra=()):
    FIN = finish(run)
    g = 'gate=finish' if gate >= FIN - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'out=' + out] + var.split() + list(extra)
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def stage(run, prefix, gate, tag, extra=()):
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda iv: (xds(run, prefix, gate, '%s_v%d' % (tag, iv[0]), iv[1], extra), '%s_v%d' % (tag, iv[0])), enumerate(VAR)))
    return sorted(r for r in res if r[0] is not None)


def apexes(T):
    ts = sorted(t for t in T if t > 1000)
    fin = min([t for t in ts if T[t]['fin']] or [max(ts)])
    out = []
    for t in ts:
        w = [T[k]['s'] for k in range(t - 15, t + 16) if k in T]
        if T[t]['s'] == min(w) and T[t]['s'] < 0.85 * max(w) and (not out or t - out[-1] > 20):
            out.append(t)
    return out, fin


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
        nf.write('- structsearch %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): structsearch %s (approach shot -> pre-fire + point-blank, x_ds + x_opt + graft)\n\n'
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
    T, _ = trace(run)
    ap, fin = apexes(T)
    if A not in ap:
        A = min(ap, key=lambda a: abs(a - A))
    nxt = [a for a in ap if a > A + 25]
    G2 = min((nxt[0] + 30) if nxt else FIN, FIN)
    fires = [t for t in sorted(T) if T[t]['fire'] and t - 1 in T and T[t - 1]['rl'] == 0 and T[t]['rl'] > 0 and t > 1000]
    # the approach shot: last shot fired before the apex (its explosion lands in the approach)
    app = [f for f in fires if A - 45 <= f <= A - 4]
    if not app:
        log('turn %d: no approach shot' % A)
        return
    F0 = app[-1]
    prev = [f for f in fires if f < F0]
    Fprev = prev[-1] if prev else 1000
    tag = os.path.join(d, 't%d' % A)
    log('turn %d (run %d): approach shot %d, previous %d, next turn gate %d' % (A, FIN, F0, Fprev, G2))
    # 1. base without the approach shot
    head(run, F0 - 12, tag + '_c0.txt')
    hi = max(F0 + 3, A - 33)
    r = stage(run, tag + '_c0.txt', A + 5, tag + '_base', ['nokick=%d,%d' % (F0 - 3, hi), 'nokickall=1'] + NEW)
    if not r:
        log('  base: NOGATE')
        return
    base = r[0][1]
    log('  base (no shot %d-%d): %s' % (F0 - 3, hi, ' '.join('%.2f' % x[0] for x in r)))
    # 2. pre-fires
    TB, _ = trace(base, A - 60)
    vx, vy = T[A + 15]['v'] if A + 15 in T else T[A]['v']
    f_lo = max(A - 32, Fprev + 25, hi + 1)
    if f_lo > A - 8:
        log('  no room for a pre-fire (%d > %d)' % (f_lo, A - 8))
        return
    out = subprocess.run([os.path.join(BIN, 'x_pfscan'), MAP, base, str(f_lo), str(A - 8), str(A - 3), str(A + 20), '40',
                          'dir=%.3f,%.3f' % (vx, vy)], capture_output=True, text=True).stdout
    cands, seen = [], set()
    for l in out.splitlines():
        m = re.match(r'fire (\d+) aim\s+([0-9.]+) -> expl (\d+) .* along ([-0-9.]+)', l)
        if not m:
            continue
        f, aim, e, along = int(m.group(1)), float(m.group(2)), int(m.group(3)), float(m.group(4))
        if along < 4 or (f, e) in seen or max(TB) < f:
            continue
        seen.add((f, e))
        cands.append((f, aim, e, along))
    log('  pre-fires: %s' % cands[:K])
    for f, aim, e, along in cands[:K]:
        ctag = '%s_f%d_%d' % (tag, f, int(aim * 10))
        L = line_at(base, f)
        if int(L[2]) and not int(line_at(base, f - 1)[2]):
            log('  pre-fire %d: hook press on that tick, skipped' % f)
            continue
        a = math.radians(aim)
        head(base, f - 1, ctag + '_p.txt', '%s %s %s 1 %d %d %s' % (L[0], L[1], L[2], round(math.cos(a) * 10000), round(math.sin(a) * 10000), L[6]))
        r1 = stage(run, ctag + '_p.txt', A + 55, ctag + '_s1', NEW)
        if not r1:
            log('  pre-fire %d/%.1f: stage 1 NOGATE' % (f, aim))
            continue
        log('  pre-fire %d/%.1f -> %d: stage 1 %s' % (f, aim, e, ' '.join('%.2f' % x[0] for x in r1)))
        l1 = r1[0][1]
        o = subprocess.run([os.path.join(BIN, 'x_opt'), MAP, 'run=' + l1, 'ref=' + run, 'cut=%d' % (F0 - 10), 'end=%d' % (A + 40),
                            'tail=20', 'iters=6000000', 'threads=%d' % CORES, 'seed=%d' % (SEED + A), 't0=0.4', 'kv=0.3',
                            'out=' + ctag + '_o.txt'], capture_output=True, text=True).stdout
        m = re.search(r'RESULT score ([-0-9.]+) lead ([-0-9.]+)', o)
        log('    x_opt: %s' % (m.group(0) if m else 'no result'))
        src = ctag + '_o.txt' if m and not 'DEAD' in o.split('RESULT')[1].split('\n')[0] else l1
        head(src, A + 30, ctag + '_c2.txt')
        r2 = stage(run, ctag + '_c2.txt', G2, ctag + '_s2', NEW)
        log('    stage 2 (%d -> %d): %s' % (A + 30, G2, ' '.join('%.2f' % x[0] for x in r2) or 'NOGATE'))
        for t2, l2 in r2[:2]:
            if t2 > G2 - 0.9:
                continue
            gd = l2 + '_mg'
            mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(A + 33), str(min(G2, FIN - 5)), '6', '3', str(CORES)],
                                capture_output=True, text=True).stdout
            log('    multigraft: %s' % ' | '.join(mg.strip().splitlines()[-2:]))
            cs = [os.path.join(gd, g) for g in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
            if G2 >= FIN:
                cs.append(l2)
            best = None
            for p in cs:
                ff = finish(p)
                if ff is not None and (best is None or ff < best[0]):
                    best = (ff, p)
            if best and best[0] < FIN and publish(best[1], best[0], 'turn %d (pre-fire %d)' % (A, f)):
                return


T0, _ = trace(BEST)
AP, FIN0 = apexes(T0)
turns = [int(x) for x in kw['turns'].split(',')] if 'turns' in kw else AP
log('structsearch on %d: turns %s' % (FIN0, turns))
for A in turns:
    try:
        try_turn(A)
    except Exception as ex:
        log('turn %d: error %r' % (A, ex))
log('done')
