#!/usr/bin/env python3
"""optsweep.py RUN DIR WIN [WIN ...] [iters=4000000] [threads=2] [par=2] [kv=0.3] [seed=1]
WIN = CUT:END:C2:G2 - x_opt polishes RUN's window CUT..END (END right after a turn exit, where speed is worth time);
its line is cut at C2 (after the exit) and re-searched by x_ds (rf variants, tracking RUN) to G2 (after the next turn);
the best stage-2 line is grafted back onto RUN (multigraft.py, every on-line cut >= 1 tick ahead, largest D first). With
WIN = CUT:END:C2:G2:G3[:G4...], a stage-2 line >= 0.9 ahead that grafts nowhere is re-searched from G2 - 8 to G3 (the
next turn: its lead is often off the run's path at G2) and grafted again, and so on through G4... while it stays ahead. A
finishing graft that passes the server check replaces RUN (DIR/best_<ticks>.txt) and the next windows use it.
This is the recipe that gave 2576 -> 2575 (NOTES "x_opt on the restructured U-turn 1"). Writes DIR/sweep.log."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
run, d = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
wins = [tuple(map(int, a.split(':'))) for a in sys.argv[3:] if a.count(':') >= 3]
kw = dict(a.split('=', 1) for a in sys.argv[3:] if '=' in a)
ITERS, TH, PAR = kw.get('iters', '4000000'), kw.get('threads', '2'), int(kw.get('par', 2))
KV, SEED = kw.get('kv', '0.3'), int(kw.get('seed', 1))
T0 = kw.get('t0', '0.4')
KVALT = kw.get('kvalt', '0,0.15')  # x_opt also keeps its best plan by these kv (0: lead only); each gets a stage 2
GALT = kw.get('galt', '0.1,0.25')  # and by lead - gw x |v - run v| (graftable leads: 2544, 2543)
VARS = kw.get('vars', 'dschain_variants_rf.txt')  # x_ds variant set for stage 2+
VAR = [l.strip() for l in open(os.path.join(HERE, VARS)) if l.strip() and not l.startswith('#')]
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
def stage(prefix, gate, tag):
    with ThreadPoolExecutor(PAR) as ex:
        res = list(ex.map(lambda iv: (xds(prefix, gate, '%s_v%d' % (tag, iv[0]), iv[1]), '%s_v%d' % (tag, iv[0])), enumerate(VAR)))
    return sorted(r for r in res if r[0] is not None)


def grafts(ok, c, g, ge):
    best = None
    for t2, l2 in ok[:2]:
        if t2 > ge - 0.9:
            continue
        gd = l2 + '_mg'
        mg = subprocess.run(['python3', os.path.join(HERE, 'multigraft.py'), l2, run, gd, str(c + 3), str(min(g, FIN - 5)), '6', '3', str(PAR)],
                            capture_output=True, text=True).stdout
        log('window %d-%d: multigraft %s: %s' % (cut, end, os.path.basename(l2), ' | '.join(mg.strip().splitlines()[-2:])))
        cands = [os.path.join(gd, f) for f in sorted(os.listdir(gd))] if os.path.isdir(gd) else []
        if g >= FIN - 3:
            cands.append(l2)
        for p in cands:
            f = finish(p)
            if f is not None and (best is None or f < best[0]):
                best = (f, p)
    return best


for k, w in enumerate(wins):
    cut, end, c2, g2 = w[:4]
    tag = os.path.join(d, 'w%d_%d' % (cut, end))
    out = subprocess.run([os.path.join(BIN, 'x_opt'), MAP, 'run=' + run, 'cut=%d' % cut, 'end=%d' % end, 'tail=20', 'iters=' + ITERS,
                          'threads=' + TH, 'seed=%d' % (SEED + k), 't0=' + T0, 'kv=' + KV, 'out=' + tag + '_o.txt'] +
                         (['kvalt=' + KVALT, 'outalt=' + tag] if KVALT else []) + (['galt=' + GALT, 'outalt=' + tag] if GALT else []), capture_output=True, text=True).stdout
    m = re.search(r'RESULT score ([-0-9.]+) lead ([-0-9.]+) lat ([0-9.]+) \|v\| ([0-9.]+) \(run ([0-9.]+)\)', out)
    log('window %d-%d: x_opt %s' % (cut, end, m.group(0) if m else 'no result'))
    alts = []
    for j, ma in enumerate(re.finditer(r'ALT kv ([0-9.]+) score ([-0-9.]+) lead ([-0-9.]+) .*', out)):
        log('window %d-%d: x_opt %s' % (cut, end, ma.group(0)))
        if float(ma.group(3)) >= 0.3 and os.path.exists('%s_k%d.txt' % (tag, j)):
            alts.append('%s_k%d.txt' % (tag, j))
    srcs = ([tag + '_o.txt'] if m and float(m.group(1)) >= 0.3 else []) + alts
    if not srcs:
        continue
    ok = []
    for j, o in enumerate(srcs):
        cj = '%s_c%s.txt' % (tag, 'a%d' % j if j else '')
        head(o, c2, cj)
        ok += stage(cj, g2, tag + ('_a%d' % j if j else ''))
    ok.sort()
    g2e = min(g2, FIN)
    log('window %d-%d: stage 2 from %d to %d: %s' % (cut, end, c2, g2e, ' '.join('%.2f' % r[0] for r in ok) or 'NOGATE'))
    best = grafts(ok, c2, g2, g2e)
    # further stages: while the lead holds (>= 0.9) but grafts nowhere, re-search through the next turn
    gp, gpe, okp = g2, g2e, ok
    for si, gn in enumerate(w[4:]):
        if (best and best[0] < FIN) or gp >= FIN - 3 or not okp or okp[0][0] > gpe - 0.9:
            break
        # two cuts: 15 ticks before the gate (sweet-maxwell: 10 before cuts into doomed approaches), and 28 before it
        # (room for a stack grenade fired ~25 ticks before its explosion, e.g. the shaft double kick: from 8 before 33
        # behind, from 30 before 6.5 behind, with surv=0 level)
        okn = []
        for cn in (gp - 15, gp - 28):
            head(okp[0][1], cn, '%s_c%d_%d.txt' % (tag, si + 3, cn))
            okn += stage('%s_c%d_%d.txt' % (tag, si + 3, cn), gn, '%s_s%d_%d' % (tag, si + 3, cn))
        okn.sort()
        cn = gp - 28
        gne = min(gn, FIN)
        log('window %d-%d: stage %d from %d / %d to %d: %s' % (cut, end, si + 3, gp - 15, gp - 28, gne, ' '.join('%.2f' % r[0] for r in okn) or 'NOGATE'))
        best = grafts(okn, cn, gn, gne)
        gp, gpe, okp = gn, gne, okn
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
