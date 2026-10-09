#!/usr/bin/env python3
"""extrashot.py DIR F0:F1:E0:E1 G1 G2 [k=4] [beam=4000] [cores=4] [vars=dschain_variants_rf_s0.txt] [dir=dx,dy]
An extra shot where the run's gun idles (Teero fires there): x_pfscan on tas-work/kog_full_best.txt's own states
(fired F0..F1, exploding E0..E1, scored along the run's velocity or dir=) -> top k distinct (fire tick, explosion
tick) -> per candidate: the run's inputs + the forced shot (a hook press on that tick is dropped) -> x_ds to the rt-G1
gate (stage 1, every variant) -> the best line cut at G1 - 15 -> x_ds to G2 (stage 2) -> multigraft onto the run ->
server check; a gain becomes kog_full_<n>.txt / kog_full_best.txt and is committed and pushed. Writes DIR/log."""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
ROOT = os.path.dirname(TW)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
BEST = os.path.join(TW, 'kog_full_best.txt')
d = os.path.abspath(sys.argv[1])
F0, F1, E0, E1 = map(int, sys.argv[2].split(':'))
G1, G2 = int(sys.argv[3]), int(sys.argv[4])
kw = dict(a.split('=', 1) for a in sys.argv[5:] if '=' in a)
K, BEAM, CORES = int(kw.get('k', 4)), kw.get('beam', '4000'), int(kw.get('cores', 4))
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf_s0.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def finish(path):
    out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '2400'], capture_output=True, text=True).stdout
    f = [int(l.split()[2]) for l in out.splitlines() if l.startswith('T ') and 'FINISH' in l]
    return min(f) if f else None


def head(path, rt, out, extra=None):
    with open(path) as f:
        lines = f.readlines()
    with open(out, 'w') as g:
        g.writelines(lines[:rt + 68])
        if extra:
            g.write(extra + '\n')


def xds(run, prefix, gate, out, var, fin):
    g = 'gate=finish' if gate >= fin - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=' + BEAM,
           'verbose=0', 'out=' + out] + var.split()
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def stage(run, prefix, gate, tag, fin):
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda iv: (xds(run, prefix, gate, '%s_v%d' % (tag, iv[0]), iv[1], fin), '%s_v%d' % (tag, iv[0])), enumerate(VAR)))
    return sorted(r for r in res if r[0] is not None)


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
        nf.write('- extrashot %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): extrashot %s (extra shot where the gun idled, x_ds re-search + graft)\n\n'
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
log('extrashot on %d: fired %d-%d exploding %d-%d, gates %d / %d, vars %s' % (FIN, F0, F1, E0, E1, G1, G2, kw.get('vars', 'rf_s0')))
pf = [os.path.join(BIN, 'x_pfscan'), MAP, run, str(F0), str(F1), str(E0), str(E1), '60'] + (['dir=' + kw['dir']] if 'dir' in kw else [])
cands, seen = [], set()
for l in subprocess.run(pf, capture_output=True, text=True).stdout.splitlines():
    m = re.match(r'fire (\d+) aim\s+([0-9.]+) -> expl (\d+) .* along ([-0-9.]+)', l)
    if m and int(m.group(3)) not in seen and float(m.group(4)) >= 3:  # the best per explosion tick
        seen.add(int(m.group(3)))
        cands.append((int(m.group(1)), float(m.group(2)), int(m.group(3)), float(m.group(4))))
log('candidates: %s' % cands[:K])
lines = open(run).readlines()
for f, aim, e, along in cands[:K]:
    tag = os.path.join(d, 'f%d_%d' % (f, int(aim * 10)))
    L = lines[f + 68 - 1].split()
    hk = L[2] if not (int(L[2]) and not int(lines[f + 68 - 2].split()[2])) else '0'
    a = math.radians(aim)
    head(run, f - 1, tag + '_p.txt', '%s %s %s 1 %d %d %s' % (L[0], L[1], hk, round(math.cos(a) * 10000), round(math.sin(a) * 10000), L[6]))
    r1 = stage(run, tag + '_p.txt', G1, tag + '_s1', FIN)
    log('fire %d aim %.1f -> %d (along %.1f): stage 1 to %d: %s' % (f, aim, e, along, G1, ' '.join('%.2f' % x[0] for x in r1) or 'NOGATE'))
    if not r1 or r1[0][0] > G1 + 3:
        continue
    head(r1[0][1], G1 - 15, tag + '_c2.txt')
    r2 = stage(run, tag + '_c2.txt', G2, tag + '_s2', FIN)
    g2e = min(G2, FIN)
    log('  stage 2 (%d -> %d): %s' % (G1 - 15, g2e, ' '.join('%.2f' % x[0] for x in r2) or 'NOGATE'))
    for t2, l2 in r2[:2]:
        if t2 > g2e - 0.9:
            continue
        gd = l2 + '_mg'
        mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(G1 - 12), str(min(G2, FIN - 5)), '6', '3', str(CORES)],
                            capture_output=True, text=True).stdout
        log('  multigraft: %s' % ' | '.join(mg.strip().splitlines()[-2:]))
        cs = [os.path.join(gd, g) for g in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
        if G2 >= FIN:
            cs.append(l2)
        best = None
        for p in cs:
            ff = finish(p)
            if ff is not None and (best is None or ff < best[0]):
                best = (ff, p)
        if best and best[0] < FIN and publish(best[1], best[0], 'fire %d aim %.1f' % (f, aim)):
            FIN = best[0]
            break
log('done')
