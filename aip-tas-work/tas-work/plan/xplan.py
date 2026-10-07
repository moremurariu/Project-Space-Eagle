#!/usr/bin/env python3
"""xplan.py: section planner - staged x_ds windows with a Pareto front (arrival time, energy) between stages.

Why: one long search over a hard section collapses at turns (the beam races into the turn and fills with lineages
that cannot exit), while short windows are about as good as the polished incumbent. And a single best state per
window (dschain) is the greedy choice that loses at turns. Here every stage is a short x_ds window from each kept
state to the incumbent's progress at the next waypoint, and the K arrivals that are non-dominated in (arrival time,
energy) go on to the next stage.

usage: xplan.py INC CUT END DIR [stage=35] [look=35] [K=6] [cores=4] [beam=3000] [variants=default|FILE]
       [stagevar=S:extra,S:extra] (extra x_ds keys for stage S only, e.g. a nokick window)
look: overlap. Each stage's windows run `look` incumbent ticks past the stage's gate and are ranked there; only their
part up to the gate (x_ds commitk=) is kept. Without it (look=0) a window spends its grenade just before the gate and
arrives where the next obstacle cannot be passed (U-turn 1: -21 at the corridor block; with look=35 see NOTES).
INC: the incumbent run (inputs from spawn); the section is INC's race ticks CUT..END, waypoints every `stage` ticks.
Writes DIR/plan.log, DIR/s<stage>_<n>.txt (arrivals) and DIR/best.txt (the earliest arrival at END's gate).
Status (NOTES "Section planner v1"): with tracking variants (variants=FILE with dschain-style trackfrac / shadow / shh
lines) it keeps up with the polished incumbent through turns (U-turn 1 apex 1.3 ahead) but loses at precision
maneuvers the incumbent got from many LNS jobs (a catch-up pre-fire on a corridor block: -21); experimental.
"""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)  # tas-work
XDS = os.environ.get('XDSBIN', os.path.join(TW, '..', 'ddnet', 'build-sim', 'x_ds'))
MAP = os.path.join(TW, 'AiP-Gores.map')

inc, cut, END, d = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
kw = dict(a.split('=', 1) for a in sys.argv[5:])
STAGE, K, CORES, BEAM = int(kw.get('stage', 35)), int(kw.get('K', 6)), int(kw.get('cores', 4)), kw.get('beam', '3000')
LOOK = int(kw.get('look', 35))
VARIANTS = [
    'seed=1 jitter=0.3 retro=4 egain=0.004',
    'seed=2 jitter=0.3 retro=4 egain=0.01',
    'seed=3 jitter=0.3 retro=3 egain=0.004 kickmin=10',
    'seed=4 jitter=0 retro=4 egain=0.02',
]
if kw.get('variants', 'default') != 'default':
    VARIANTS = [l.strip() for l in open(kw['variants']) if l.strip() and not l.startswith('#')]
STAGEVAR = {}
for sv in filter(None, kw.get('stagevar', '').split(',')):
    s, extra = sv.split(':', 1)
    STAGEVAR[int(s)] = extra
os.makedirs(d, exist_ok=True)
inc = os.path.abspath(inc)
LOG = open(os.path.join(d, 'plan.log'), 'a')


def log(m):
    m = time.strftime('%H:%M:%S ') + m
    print(m, flush=True)
    LOG.write(m + '\n')
    LOG.flush()


def window(prefix, gate, out, extra):
    """x_ds window from prefix; ranked at gate+LOOK, the kept prefix is the line up to gate (out.c)"""
    far = min(gate + LOOK, END) if LOOK > 0 else gate
    args = [XDS, MAP, f'inc={inc}', f'prefix={os.path.abspath(prefix)}', f'gate=rt{far}', 'incforce=0', 'threads=1',
            'verbose=0', f'beam={BEAM}', f'out={os.path.abspath(out)}'] + extra.split()
    if far > gate:
        args.append(f'commitk={gate}')
    r = subprocess.run(args, capture_output=True, text=True, cwd=TW).stdout
    m = re.search(r'GATE t ([\d.]+) \(incumbent ([\d.]+)\) E (-?\d+)', r)
    v = re.search(r'verify \(CTasGame\): gate rt (-?\d+) end rt (-?\d+) geo \S+ finish (-?\d+) dead (\d)', r)
    keep = out + '.c' if far > gate else out
    if not m or not v or v.group(4) != '0' or int(v.group(1)) < 0 or not os.path.exists(keep):
        return None
    return float(m.group(1)), float(m.group(2)), int(m.group(3)), keep


def pareto(arr, k):
    """non-dominated in (time low, energy high), then the earliest others up to k"""
    arr = sorted(arr, key=lambda a: (a[0], -a[2]))
    front, best_e = [], -1e18
    for a in arr:
        if a[2] > best_e + 1:
            front.append(a)
            best_e = a[2]
    rest = [a for a in arr if a not in front]
    keep = front[:k]
    for a in rest:
        if len(keep) >= k:
            break
        keep.append(a)
    return sorted(keep, key=lambda a: a[0])


# the starting prefix: INC cut at CUT
start = os.path.join(d, 'start.txt')
lines = open(inc).read().splitlines()
open(start, 'w').write('\n'.join(lines[:cut + 68]) + '\n')
gates = list(range(cut + STAGE, END, STAGE)) + [END]
kept = [(0.0, 0.0, 0, start)]
log(f'xplan inc={inc} cut={cut} end={END} gates={gates} look={LOOK} K={K} beam={BEAM} variants={len(VARIANTS)}')
for si, g in enumerate(gates):
    jobs = []
    for ki, (_, _, _, p) in enumerate(kept):
        for vi, v in enumerate(VARIANTS):
            extra = v + (' ' + STAGEVAR[si] if si in STAGEVAR else '')
            jobs.append((p, g, os.path.join(d, f's{si}_k{ki}_v{vi}.txt'), extra))
    t0 = time.time()
    with ThreadPoolExecutor(CORES) as ex:
        res = list(ex.map(lambda j: window(*j), jobs))
    arr, seen = [], set()
    for r in res:  # variants from one prefix often commit the same line
        if r and (h := hash(open(r[3]).read())) not in seen:
            seen.add(h)
            arr.append(r)
    if not arr:
        log(f'stage {si} (gate rt{g}): no arrival')
        sys.exit(1)
    inc_t = arr[0][1]
    kept = pareto(arr, K)
    log(f'stage {si} gate rt{g} (ranked at rt{min(g + LOOK, END) if LOOK else g}, incumbent {inc_t:.2f}): {len(arr)}/{len(jobs)} arrivals in {time.time()-t0:.0f}s; kept ' +
        ' '.join(f'[{a[0]:.2f} E{a[2]}]' for a in kept))
best = kept[0]
open(os.path.join(d, 'best.txt'), 'w').write(open(best[3]).read())
log(f'BEST arrival at rt{END}: {best[0]:.3f} (incumbent {best[1]:.3f}) E {best[2]} -> {best[3]}')
