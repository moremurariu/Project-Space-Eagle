#!/usr/bin/env python3
"""optsweep.py RUN DIR WIN [WIN ...] [iters=4000000] [threads=2] [par=2] [kv=0.3] [seed=1] [vars=FILE in plan/] [chain=4]
WIN = CUT:END:C2:G2 - x_opt polishes RUN's window CUT..END (END right after a turn exit, where speed is worth time);
its line is cut at C2 (after the exit) and re-searched by x_ds (rf variants, tracking RUN) to G2 (after the next turn);
the best stage-2 line is grafted back onto RUN (multigraft.py, every on-line cut >= 1 tick ahead, largest D first); a
stage-2 line only a fraction of a tick ahead (>= 0.25) is chained on through the next turns (chain=N stages). A
finishing graft that passes the server check replaces RUN (DIR/best_<ticks>.txt) and the next windows use it.
This is the recipe that gave 2576 -> 2575 (NOTES "x_opt on the restructured U-turn 1"). Writes DIR/sweep.log."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
run, d = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
wins = [tuple(map(int, a.split(':'))) for a in sys.argv[3:] if a.count(':') == 3]
kw = dict(a.split('=', 1) for a in sys.argv[3:] if '=' in a)
ITERS, TH, PAR = kw.get('iters', '4000000'), kw.get('threads', '2'), int(kw.get('par', 2))
KV, SEED = kw.get('kv', '0.3'), int(kw.get('seed', 1))
T0 = kw.get('t0', '0.4')
VAR = [l.strip() for l in open(os.path.join(HERE, kw.get('vars', 'dschain_variants_rf.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'sweep.log'), 'a')


def log(s):
    LOG.write(time.strftime('%H:%M:%S ') + s + '\n')
    LOG.flush()


def finish(path):
    out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '2400'], capture_output=True, text=True).stdout
    f = [int(l.split()[2]) for l in out.splitlines() if l.startswith('T ') and 'FINISH' in l]
    return min(f) if f else None


def head(path, rt, out):
    with open(path) as f:
        lines = f.readlines()
    with open(out, 'w') as g:
        g.writelines(lines[:rt + 68])


def xds(prefix, gate, out, var):
    g = 'gate=finish' if gate >= FIN - 3 else 'gate=rt%d' % gate
    cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + prefix, g, 'incforce=0', 'threads=1', 'beam=4000',
           'verbose=0', 'out=' + out] + var.split()
    with open(out + '.log', 'w') as lf:
        subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(out + '.log').read())
    return float(m.group(1)) if m else None


def publish(path, ticks, what):
    """copy a server-checked gain to kog_full_<n>.txt / kog_full_best.txt (if better) and commit + push it"""
    ROOT = os.path.dirname(TW)
    BEST = os.path.join(TW, 'kog_full_best.txt')
    cur = finish(BEST)
    if cur is not None and ticks >= cur:
        return
    dst = os.path.join(TW, 'kog_full_%d.txt' % ticks)
    subprocess.run(['cp', path, dst])
    subprocess.run(['cp', path, BEST])
    with open(os.path.join(TW, 'NOTES.md'), 'a') as nf:
        nf.write('- %s (%s): **%d (%.2f s)**, server-checked.\n' % (what, os.path.relpath(path, TW), ticks, ticks / 50.0))
    msg = ('Full run %d (%.2f s): %s (x_opt + x_ds re-search + graft)\n\nServer-checked: TasReplay, no freeze, no double start.\n\n'
           'Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>\n'
           'Claude-Session: https://claude.ai/code/session_01XxupAVqSvwb6MWzmsVwkue') % (ticks, ticks / 50.0, what)
    subprocess.run(['git', '-C', ROOT, 'add', os.path.relpath(dst, ROOT), 'tas-work/kog_full_best.txt', 'tas-work/NOTES.md'])
    subprocess.run(['git', '-C', ROOT, 'commit', '-q', '-m', msg])
    for k in range(4):
        if subprocess.run(['git', '-C', ROOT, 'push', '-u', 'origin', 'claude/sweet-maxwell-3t11yw']).returncode == 0:
            break
        time.sleep(2 ** (k + 1))


CHAIN = int(kw.get('chain', 4))


def apexes(path):
    T = {}
    for l in subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, '990'], capture_output=True, text=True).stdout.splitlines():
        a = l.split()
        if len(a) > 7 and a[0] == 'T':
            T[int(a[2])] = float(a[7])
    ts = sorted(t for t in T if t > 1000)
    out = []
    for t in ts:
        w = [T[k] for k in range(t - 15, t + 16) if k in T]
        if T[t] == min(w) and T[t] < 0.85 * max(w) and (not out or t - out[-1] > 20):
            out.append(t)
    return out


def graft(l2, c0, g, tag):
    """multigraft l2 onto run from c0 to g; the finishing candidates (l2 itself when g is the finish)"""
    gd = l2 + '_mg'
    mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(c0), str(min(g, FIN - 5)), '6', '3', str(PAR)],
                        capture_output=True, text=True).stdout
    log('%s: multigraft %s: %s' % (tag, os.path.basename(l2), ' | '.join(mg.strip().splitlines()[-2:])))
    cands = [os.path.join(gd, x) for x in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
    if g >= FIN - 3:
        cands.append(l2)
    best = None
    for p in cands:
        f = finish(p)
        if f is not None and (best is None or f < best[0]):
            best = (f, p)
    return best


def chain(line, gate, tag):
    """a line ahead of the run by a fraction of a tick at its gate is not graftable (a graft needs a whole tick): it is
    re-searched on from gate - 10 to the gate after the next turn, stage by stage, until its lead makes a graft (or
    the finish) faster, or the lead falls below 0.2 (NOTES: chained leads)"""
    ap = apexes(run)
    for k in range(CHAIN):
        nx = [a for a in ap if a > gate + 15]
        g = min(nx[0] + 30 if nx else FIN, FIN)
        if g - gate < 40:
            g = min(gate + 80, FIN)
        cut = gate - 10
        ctag = '%s_ch%d' % (tag, k)
        head(line, cut, ctag + '_c.txt')
        with ThreadPoolExecutor(PAR) as ex:
            res = list(ex.map(lambda iv: (xds(ctag + '_c.txt', g, '%s_v%d' % (ctag, iv[0]), iv[1]), '%s_v%d' % (ctag, iv[0])), enumerate(VAR)))
        ok = sorted(r for r in res if r[0] is not None)
        ge = min(g, FIN)
        log('%s: chain %d from %d to %d: %s' % (os.path.basename(tag), k, cut, ge, ' '.join('%.2f' % r[0] for r in ok) or 'NOGATE'))
        if not ok or ge - ok[0][0] < 0.2:
            return None
        t2, l2 = ok[0]
        if ge - t2 >= 0.9 or g >= FIN - 3:
            b = graft(l2, cut + 3, g, os.path.basename(tag))
            if b and b[0] < FIN:
                return b
        line, gate = l2, g
    return None


FIN = finish(run)
log('optsweep on %s (finish %s), windows %s' % (run, FIN, wins))
for k, (cut, end, c2, g2) in enumerate(wins):
    tag = os.path.join(d, 'w%d_%d' % (cut, end))
    out = subprocess.run([os.path.join(BIN, 'x_opt'), MAP, 'run=' + run, 'cut=%d' % cut, 'end=%d' % end, 'tail=20', 'iters=' + ITERS,
                          'threads=' + TH, 'seed=%d' % (SEED + k), 't0=' + T0, 'kv=' + KV, 'out=' + tag + '_o.txt'], capture_output=True, text=True).stdout
    m = re.search(r'RESULT score ([-0-9.]+) lead ([-0-9.]+) lat ([0-9.]+) \|v\| ([0-9.]+) \(run ([0-9.]+)\)', out)
    log('window %d-%d: x_opt %s' % (cut, end, m.group(0) if m else 'no result'))
    if not m or float(m.group(1)) < 0.3:
        continue
    head(tag + '_o.txt', c2, tag + '_c.txt')
    with ThreadPoolExecutor(PAR) as ex:
        res = list(ex.map(lambda iv: (xds(tag + '_c.txt', g2, '%s_v%d' % (tag, iv[0]), iv[1]), '%s_v%d' % (tag, iv[0])), enumerate(VAR)))
    ok = sorted(r for r in res if r[0] is not None)
    g2e = min(g2, FIN)
    log('window %d-%d: stage 2 from %d to %d: %s' % (cut, end, c2, g2e, ' '.join('%.2f' % r[0] for r in ok) or 'NOGATE'))
    best = None
    for t2, l2 in ok[:2]:
        if t2 > g2e - 0.9:
            continue
        b = graft(l2, c2 + 3, g2, 'window %d-%d' % (cut, end))
        if b and (best is None or b[0] < best[0]):
            best = b
    if (best is None or best[0] >= FIN) and ok and g2e - ok[0][0] >= 0.25 and g2 < FIN - 3 and CHAIN > 0:
        b = chain(ok[0][1], g2, tag)
        if b and (best is None or b[0] < best[0]):
            best = b
    if best and best[0] < FIN:
        chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), best[1]], capture_output=True, text=True).stdout
        mm = re.search(r'-> (\d+) ticks', chk)
        if mm and int(mm.group(1)) == best[0] and 'frozen ticks 0' in chk:
            subprocess.run(['cp', best[1], os.path.join(d, 'best_%d.txt' % best[0])])
            subprocess.run(['cp', best[1], run])
            FIN = best[0]
            log('window %d-%d: NEW BEST %d (server-checked) from %s' % (cut, end, best[0], best[1]))
            if '/runs/loop/' in d:
                publish(best[1], best[0], 'optloop window %d-%d' % (cut, end))
        else:
            log('window %d-%d: graft %d failed the server check: %s' % (cut, end, best[0], chk.strip().replace('\n', ' | ')))
log('done')
