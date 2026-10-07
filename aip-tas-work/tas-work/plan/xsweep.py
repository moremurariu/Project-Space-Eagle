#!/usr/bin/env python3
"""xsweep.py: section planner + graft over a list of sections, carrying every gain into the next section.

For each section CUT:END of the current best run: xplan.py (overlapping stages) plans the section; each of the best 3
kept arrivals that is ahead of the run at END (by >= 0.5 ticks, energy-adjusted) is grafted back onto the run with
x_graft from the latest two ticks where it is on the run's line (within `off` px) and >= 1 tick ahead, with D = the
whole ticks ahead there (and one less): if the planned line is the run's line some ticks earlier, the graft puts the
run's remaining inputs on it. The best finishing graft that passes the server check (TasReplay) becomes the run.
This is how 2606 -> 2605 was found (U-turn 1, cut 1095 -> 1270, graft D=1 from rt 1250).

usage: xsweep.py RUN DIR CUT:END [CUT:END ...] [cores=4] [off=6] [rounds=1] [seed0=0] [K=6] [look=35] [stage=35]
       [variants=FILE]
rounds: repeat the section list with new seeds (xplan seedadd=100*round); a re-plan of a section on a new best run
usually differs anyway (U-turn 1 gave a tick on three re-plans).
Writes DIR/sweep.log, DIR/sec<i>/ (xplan), DIR/sec<i>/g_*.txt (grafts) and DIR/best_<ticks>.txt for every gain."""
import math, os, re, shutil, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')

run, d = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
secs = [tuple(map(int, a.split(':'))) for a in sys.argv[3:] if ':' in a and '=' not in a]
kw = dict(a.split('=', 1) for a in sys.argv[3:] if '=' in a)
CORES = int(kw.get('cores', 4))
ROUNDS = int(kw.get('rounds', 1))
SEED0 = int(kw.get('seed0', 0))  # seed offset (xplan seedadd = seed0 + 100 * round)
LOOK = int(kw.get('look', 35))
OFF = float(kw.get('off', 6))
plan_kw = [f'{k}={v}' for k, v in kw.items() if k in ('K', 'look', 'stage', 'beam')]
plan_kw.append('variants=' + os.path.abspath(kw.get('variants', os.path.join(HERE, 'dschain_variants.txt'))))
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'sweep.log'), 'a')


def log(m):
    m = time.strftime('%H:%M:%S ') + m
    print(m, flush=True)
    LOG.write(m + '\n')
    LOG.flush()


def server_finish(path):
    """race ticks on the server code (TasReplay), or None if it freezes / does not finish"""
    r = subprocess.run(['bash', os.path.join(TW, 'srvfin.sh'), path], capture_output=True, text=True).stdout
    m = re.search(r'-> (\d+) ticks', r)
    f = re.search(r'frozen ticks (\d+)', r)
    if not m or not f or f.group(1) != '0' or 'DOUBLE' in r:
        return None
    return int(m.group(1))


def graft(prefix, cut, D, out):
    r = subprocess.run([os.path.join(BIN, 'x_graft'), MAP, f'prefix={prefix}', f'cut={cut}', f'run={run}', f'D={D}',
                        'horizon=45', 'beam=20000', 'threads=2', 'test=300', f'out={out}'],
                       capture_output=True, text=True, cwd=TW).stdout
    m = re.search(r'RESULT graft finish (\d+)', r)
    return (int(m.group(1)), out) if m else None


def traj(path, rt0):
    """race tick -> (x, y) from x_trace"""
    out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, path, str(rt0)], capture_output=True, text=True).stdout
    T = {}
    for l in out.splitlines():
        a = l.split()
        if len(a) > 4 and a[0] == 'T':
            T[int(a[2])] = (float(a[3]), float(a[4]))
    return T


def leads(A, B, r0, r1):
    """A's lead over B (ticks) and distance to B's path at A's race ticks r0..r1 (projection onto B's path)"""
    res, m = {}, None
    for t in range(r0, r1 + 1):
        if t not in A:
            break
        p, best = A[t], None
        for r in (range(t - 30, t + 30) if m is None else range(int(m) - 8, int(m) + 12)):
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
        res[t] = (m - t, best[0])
    return res


cur = server_finish(run)
log(f'xsweep run={run} ({cur}) sections={secs} plan={" ".join(plan_kw)}')
for i, (rnd, (cut, end)) in enumerate((r, s) for r in range(ROUNDS) for s in secs):
    sd = os.path.join(d, f'sec{i}_{cut}_{end}')
    t0 = time.time()
    fin = os.path.join(sd, 'final.txt')
    if not os.path.exists(fin):  # (a rerun reuses a finished plan)
        subprocess.run([sys.executable, os.path.join(HERE, 'xplan.py'), run, str(cut), str(end), sd, f'cores={CORES}',
                        f'seedadd={SEED0 + 100 * rnd}'] + plan_kw, capture_output=True, text=True)
    if not os.path.exists(fin):
        log(f'section {cut}-{end}: planner failed ({time.time()-t0:.0f}s)')
        continue
    arr = [(float(a), int(e), f, float(r)) for a, e, f, r in (l.split() for l in open(fin))]
    ahead = [a for a in arr if a[3] - a[0] >= 0.5]
    log(f'section {cut}-{end} (round {rnd}): best {arr[0][0]:.2f} (run {arr[0][3]:.0f}), {len(ahead)} ahead ({time.time()-t0:.0f}s)')
    # graft cuts: ticks where the planned line is on the run's line (within OFF px) and ahead by >= 1 tick; the two
    # latest such ticks (spaced >= 10) of each of the best 3 arrivals, D = whole ticks ahead there and one less
    B = traj(run, cut)
    jobs = []
    for ai, (t, e, f, _) in enumerate(ahead[:3]):
        # the window's whole line (its look-ahead past END too: the ranked gain is often there), not only the kept part
        if f.endswith('.c') and os.path.exists(f[:-2]):
            f = f[:-2]
        endl = end + LOOK if f.endswith('.txt') and not f.endswith('.c') else end
        L = leads(traj(f, cut), B, cut, endl)
        ok = [c for c in sorted(L, reverse=True) if c <= endl - 5 and L[c][1] < OFF and L[c][0] >= 1.0]
        cuts = []
        for c in ok:
            if all(abs(c - c2) >= 10 for c2 in cuts):
                cuts.append(c)
            if len(cuts) == 2:
                break
        log(f'  arrival {ai} ({t:.2f}): lead ' + ' '.join(f'{c}:{L[c][0]:+.1f}/{L[c][1]:.0f}px' for c in sorted(L)
                                                     if (c - cut) % 15 == 0 or c == max(L)) + f' | graft cuts {cuts}')
        for gc in cuts:
            for D in sorted({int(L[gc][0]), int(L[gc][0]) - 1} - {0}, reverse=True):
                jobs.append((f, gc, D, os.path.join(sd, f'g_a{ai}_c{gc}_D{D}.txt')))
    with ThreadPoolExecutor(max(1, CORES // 2)) as ex:
        res = [r for r in ex.map(lambda j: graft(*j), jobs) if r]
    res.sort()
    log(f'  grafts: {len(res)}/{len(jobs)} finish' + (f', best {res[0][0]}' if res else ''))
    for fn, path in res:
        if cur is not None and fn >= cur:
            break
        srv = server_finish(path)
        log(f'  {os.path.basename(path)}: {fn} -> server {srv}')
        if srv is not None and (cur is None or srv < cur):
            cur, run = srv, os.path.join(d, f'best_{srv}.txt')
            shutil.copy(path, run)
            log(f'  NEW BEST {srv} -> {run}')
            break
log(f'done: {cur} -> {run}')
