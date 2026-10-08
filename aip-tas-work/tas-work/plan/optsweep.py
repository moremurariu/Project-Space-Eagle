#!/usr/bin/env python3
"""optsweep.py RUN DIR WIN [WIN ...] [iters=4000000] [threads=2] [par=2] [kv=0.3] [seed=1]
WIN = CUT:END:C2:G2 - x_opt polishes RUN's window CUT..END (END right after a turn exit, where speed is worth time);
its line is cut at C2 (after the exit) and re-searched by x_ds (rf variants, tracking RUN) to G2 (after the next turn);
the best stage-2 line is grafted back onto RUN (multigraft.py, every on-line cut >= 1 tick ahead, largest D first). A
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
VAR = [l.strip() for l in open(os.path.join(HERE, 'dschain_variants_rf.txt')) if l.strip() and not l.startswith('#')]
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


FIN = finish(run)
log('optsweep on %s (finish %s), windows %s' % (run, FIN, wins))
for k, (cut, end, c2, g2) in enumerate(wins):
    tag = os.path.join(d, 'w%d_%d' % (cut, end))
    out = subprocess.run([os.path.join(BIN, 'x_opt'), MAP, 'run=' + run, 'cut=%d' % cut, 'end=%d' % end, 'tail=20', 'iters=' + ITERS,
                          'threads=' + TH, 'seed=%d' % (SEED + k), 't0=0.4', 'kv=' + KV, 'out=' + tag + '_o.txt'], capture_output=True, text=True).stdout
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
        gd = l2 + '_mg'
        mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(c2 + 3), str(min(g2, FIN - 5)), '6', '3', str(PAR)],
                            capture_output=True, text=True).stdout
        log('window %d-%d: multigraft %s: %s' % (cut, end, os.path.basename(l2), ' | '.join(mg.strip().splitlines()[-2:])))
        cands = [os.path.join(gd, g) for g in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
        if g2 >= FIN - 3:
            cands.append(l2)
        for p in cands:
            f = finish(p)
            if f is not None and (best is None or f < best[0]):
                best = (f, p)
    if best and best[0] < FIN:
        chk = subprocess.run([os.path.join(TW, 'srvfin.sh'), best[1]], capture_output=True, text=True).stdout
        mm = re.search(r'-> (\d+) ticks', chk)
        if mm and int(mm.group(1)) == best[0] and 'frozen ticks 0' in chk:
            subprocess.run(['cp', best[1], os.path.join(d, 'best_%d.txt' % best[0])])
            subprocess.run(['cp', best[1], run])
            FIN = best[0]
            log('window %d-%d: NEW BEST %d (server-checked) from %s' % (cut, end, best[0], best[1]))
        else:
            log('window %d-%d: graft %d failed the server check: %s' % (cut, end, best[0], chk.strip().replace('\n', ' | ')))
log('done')
