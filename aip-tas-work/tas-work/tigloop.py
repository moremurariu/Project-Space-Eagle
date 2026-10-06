#!/usr/bin/env python3
"""Tracker-seeded improvement loop. For segment starts s along the incumbent: run x_tig (follow Teero's video run) from
the incumbent's state at s, measure its lead over the incumbent (x_ds root progress) at cut points, rejoin the
incumbent's line from the best cuts with short x_ds windows, then chain to the finish (dschain, prefix0 = the rejoin).
A finish better than the incumbent's is checked on the real server (TasReplay) and becomes the new incumbent.
usage: tigloop.py INC NAME [starts=1800,1600,...] [seg=220] [tbeam=3000] [cuts=3] [rgate=100] [hours=3] [workers=4]"""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
B = '../ddnet/build-sim/'
inc, name = sys.argv[1], sys.argv[2]
kw = dict(a.split('=', 1) for a in sys.argv[3:])
SEG, TBEAM, NCUT, RGATE = int(kw.get('seg', 220)), kw.get('tbeam', '3000'), int(kw.get('cuts', 3)), int(kw.get('rgate', 100))
NW = int(kw.get('workers', 4))
T_END = time.time() + float(kw.get('hours', 3)) * 3600
D = f'runs/tig/{name}'
os.makedirs(D, exist_ok=True)
LOG = f'{D}/loop.log'
CSV, TRACK = 'teero/teero_inputs_0-3131.csv', 'teero_track.txt'
TIGARGS = ['tolf=3', 'tolj=2', 'pendwin=6', 'lagw=0.3', 'pendb=4', 'freeb=2', 'warp=2', 'every=1000']


def log(s):
    print(s, flush=True)
    with open(LOG, 'a') as f:
        f.write(time.strftime('%H:%M:%S ') + s + '\n')


def sh(args, timeout=None):
    return subprocess.run(args, capture_output=True, text=True, timeout=timeout).stdout


def trace(run):
    T = {}
    for l in sh([B + 'x_trace', 'AiP-Gores.map', run]).splitlines():
        a = l.split()
        if a and a[0] == 'T':
            T[int(a[2])] = (float(a[3]), float(a[4]), float(a[5]), float(a[6]))
    return T


TRK = {}
for l in open(TRACK):
    a = l.split()
    if len(a) >= 3:
        TRK[int(a[0])] = (float(a[1]), float(a[2]))


def teero_labels(T, rt0, rt1, lab0):
    """monotone projection of a run's positions onto Teero's track: rt -> label"""
    lab, out = lab0, {}
    for rt in range(rt0, rt1 + 1):
        if rt not in T:
            continue
        q = T[rt]
        best, bl = 1e18, None
        for a in range(int(lab) - 3, int(lab) + 25):
            if a not in TRK or a + 1 not in TRK:
                continue
            A, Bp = TRK[a], TRK[a + 1]
            abx, aby = Bp[0] - A[0], Bp[1] - A[1]
            l2 = abx * abx + aby * aby
            f = max(0, min(1, ((q[0] - A[0]) * abx + (q[1] - A[1]) * aby) / l2)) if l2 else 0
            d = math.hypot(q[0] - A[0] - abx * f, q[1] - A[1] - aby * f)
            if d < best:
                best, bl = d, a + f
        if bl is not None and best < 60:
            lab = max(lab, bl)
        out[rt] = lab
    return out


def prefix(run, rt, path):
    L = [l for l in open(run).read().splitlines() if l.strip()]
    open(path, 'w').write('\n'.join(L[:rt + 68]) + '\n')
    return path


def root_progress(pre):
    r = sh([B + 'x_ds', 'AiP-Gores.map', f'inc={inc}', f'prefix={pre}', 'gate=finish', 'maxsteps=0', 'verbose=0'])
    m = re.search(r'root progress ([\d.]+)', r)
    return float(m.group(1)) if m else -1


def finish_of(run):
    out = sh(['./srvfin.sh', run])
    m = re.search(r'finish tick (\d+) -> (\d+) ticks', out)
    ok = m and 'frozen ticks 0' in out and 'DOUBLE' not in out and 'died' not in out and '[       OK ]' in out
    return int(m.group(2)) if ok else None


F = finish_of(inc)
log(f'tigloop {name}: incumbent {inc} server finish {F}')
IT = trace(inc)
LAB = teero_labels(IT, 975, max(IT), 982.0)
starts = [int(x) for x in kw.get('starts', '1800').split(',')]
for s in starts:
    if time.time() > T_END:
        break
    off = LAB[s] - s
    tg = f'{D}/s{s}_tig.txt'
    t0 = time.time()
    r = sh([B + 'x_tig', 'AiP-Gores.map', prefix(inc, s, f'{D}/s{s}_pre.txt'), f'csv={CSV}', f'track={TRACK}', f'off={off:.1f}',
            f'beam={TBEAM}', f'maxt={s + SEG}', f'out={tg}'] + TIGARGS)
    # lead over the incumbent at cuts every 10 ticks
    leads = []
    for c in range(s + 20, s + SEG, 10):
        p = root_progress(prefix(tg, c, f'{D}/s{s}_c{c}.txt'))
        if p > 0:
            leads.append((p - c, c))
    log(f'start {s} (off {off:.1f}): tracker {time.time() - t0:.0f}s, leads ' + ' '.join(f'{c}:{l:+.1f}' for l, c in leads))
    good = sorted([x for x in leads if x[0] >= 1.0], reverse=True)[:NCUT]
    if not good:
        continue

    def rejoin(lc):
        l, c = lc
        out = f'{D}/s{s}_c{c}_rj.txt'
        r = sh([B + 'x_ds', 'AiP-Gores.map', f'inc={inc}', f'prefix={D}/s{s}_c{c}.txt', f'gate=rt{c + RGATE}', 'beam=1500', 'threads=1',
                'verbose=0', 'surv=16', 'shadow=2', 'trackfrac=0.3', f'out={out}'])
        m = re.search(r'GATE t ([\d.]+) \(incumbent ([\d.]+)\)', r)
        mv = re.search(r'verify \(CTasGame\): gate rt (-?\d+) end rt (-?\d+) geo \S+ finish (-?\d+) dead (\d)', r)
        if not m or not mv or mv.group(4) != '0':
            return (-99, c, out)
        return (float(m.group(2)) - float(m.group(1)), c, out)

    with ThreadPoolExecutor(NW) as ex:
        R = sorted(ex.map(rejoin, good), reverse=True)
    log(f'  rejoins (gate +{RGATE}): ' + ' '.join(f'{c}:{l:+.2f}' for l, c, _ in R))
    if R[0][0] < 0.5:
        continue
    l, c, rj = R[0]
    gate_rt = int(re.search(r'end rt (\d+)', sh([B + 'x_ds', 'AiP-Gores.map', f'inc={inc}', f'prefix={rj}', 'gate=finish', 'maxsteps=0', 'verbose=0'])).group(1)) \
        if False else None
    tag = f'{name}_s{s}'
    out = sh(['python3', 'dschain.py', inc, str(c), tag, f'prefix0={rj}', 'sinks=auto', 'n=4', f'workers={NW}', 'beam=1000', 'egain=0.002'])
    m = re.search(r'FINISH (\d+) \(incumbent (\d+)\) -> (\S+)', out)
    if not m:
        log(f'  chain {tag}: no finish')
        continue
    fin, run = int(m.group(1)), m.group(3)
    log(f'  chain {tag}: finish {fin} (incumbent {F})')
    if fin < F:
        sf = finish_of(run)
        if sf is not None and sf < F:
            F = sf
            newb = f'kog_full_{F}.txt'
            subprocess.run(['cp', run, newb])
            log(f'NEW BEST {F} (server-checked) -> {newb}')
            inc = newb
            IT = trace(inc)
            LAB = teero_labels(IT, 975, max(IT), 982.0)
        else:
            log(f'  server check failed / not better: {sf}')
