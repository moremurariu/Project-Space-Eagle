#!/usr/bin/env python3
"""Tracker-seeded improvement loop. For each segment start s along the incumbent:
 1. x_tig follows Teero's video run (position-indexed hints) from the incumbent's state at s, for SEG ticks, with
    several seeds; the lead over the incumbent (x_ds root progress) is measured at cut points every 10 ticks;
 2. from the best (seed, cut), x_tig follows the incumbent itself (run2ref.py reference) to the finish, several seeds:
    a full run T that keeps the lead for a while;
 3. dschain from the cut with T as forced lineage (prefix0/anc0) and the incumbent as reference, to the finish.
A finish better than the incumbent's is checked on the real server (TasReplay) and becomes the new incumbent.
usage: tigloop.py INC NAME [starts=1800,2000,...] [seg=300] [seeds=4] [minlead=2] [hours=4]"""
import math, os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
B = '../ddnet/build-sim/'
inc, name = sys.argv[1], sys.argv[2]
kw = dict(a.split('=', 1) for a in sys.argv[3:])
SEG, NSEED, MINLEAD = int(kw.get('seg', 300)), int(kw.get('seeds', 4)), float(kw.get('minlead', 0.5))
JUDGE = int(kw.get('judge', 150))
NPICK = int(kw.get('pick', 8))
SEED0 = int(kw.get('seed0', 0))  # seed offset: re-runs explore other tie-breaks
TIGX = kw.get('tigx', '').split()  # extra x_tig args for the Teero-tracker stage
T_END = time.time() + float(kw.get('hours', 4)) * 3600
D = f'runs/tig/{name}'
os.makedirs(D, exist_ok=True)
LOG = f'{D}/loop.log'
CSV, TRACK = 'teero/teero_inputs_0-3131.csv', 'teero_track.txt'
TIG = ['beam=3000', 'every=1000', 'tolf=3', 'tolj=2', 'pendwin=6', 'lagw=0.3', 'pendb=4', 'freeb=2', 'threads=1']


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
            T[int(a[2])] = (float(a[3]), float(a[4]))
    return T


TRK = {}
for l in open(TRACK):
    a = l.split()
    if len(a) >= 3:
        TRK[int(a[0])] = (float(a[1]), float(a[2]))


def teero_labels(T):
    """monotone projection of a run's positions onto Teero's track: rt -> label"""
    lab, out = 982.0, {}
    for rt in sorted(T):
        if rt < 971:
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


def cut(run, rt, path):
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


def tig(pre, csv, track, off, maxt, out, seed, extra=()):
    seed += SEED0
    j = '0' if seed == 0 else '3'
    r = sh([B + 'x_tig', 'AiP-Gores.map', pre, f'csv={csv}', f'track={track}', f'off={off:.1f}', f'maxt={maxt}', f'out={out}',
            f'seed={seed}', f'jitter={j}'] + TIG + list(extra))
    m = re.search(r'finish (-?\d+)', r)
    return int(m.group(1)) if m else -1


F = finish_of(inc)
log(f'tigloop {name}: incumbent {inc} server finish {F}')


def refresh():
    global LAB
    LAB = teero_labels(trace(inc))
    sh(['python3', 'run2ref.py', inc, f'{D}/inc_track.txt', f'{D}/inc.csv'])


refresh()
starts = [int(x) for x in kw.get('starts', '1800').split(',')]
for s in starts:
    if time.time() > T_END:
        break
    t0 = time.time()
    off = LAB[s] - s
    pre = cut(inc, s, f'{D}/s{s}_pre.txt')
    # 1. follow Teero
    with ThreadPoolExecutor(NSEED) as ex:
        list(ex.map(lambda sd: tig(pre, CSV, TRACK, off, s + SEG, f'{D}/s{s}_t{sd}.txt', sd, TIGX), range(NSEED)))
    cands = []
    for sd in range(NSEED):
        tf = f'{D}/s{s}_t{sd}.txt'
        if not os.path.exists(tf):
            continue
        n = sum(1 for l in open(tf) if l.strip())
        for c in range(s + 20, min(s + SEG, n - 68), 10):
            p = root_progress(cut(tf, c, f'{D}/s{s}_t{sd}_c.txt'))
            if p > 0:
                cands.append((p - c, sd, c))
    cands.sort(reverse=True)
    # the best few cuts (at least 30 ticks apart) are judged by a short follower run: a position lead at the cut can
    # hide a state that loses it right after
    pick = []
    for l, sd, c in cands:
        if l < MINLEAD or len(pick) >= NPICK:
            break
        if all(abs(c - c2) >= 15 for _, _, c2 in pick):
            pick.append((l, sd, c))
    log(f'start {s} (off {off:.1f}): Teero-tracker cuts ' + ' '.join(f'{c}(s{sd}):{l:+.1f}' for l, sd, c in pick) +
        f' ({time.time() - t0:.0f}s)')
    if not pick:
        continue

    def judge(x):
        l, sd, c = x
        pre2 = cut(f'{D}/s{s}_t{sd}.txt', c, f'{D}/s{s}_c{c}.txt')
        out = f'{D}/s{s}_c{c}_j.txt'
        tig(pre2, f'{D}/inc.csv', f'{D}/inc_track.txt', l, c + JUDGE, out, 0, ['shadow=' + inc])
        n = sum(1 for z in open(out) if z.strip())
        e = min(c + JUDGE, n - 68)
        return (root_progress(cut(out, e, f'{D}/s{s}_c{c}_je.txt')) - e, l, sd, c)

    with ThreadPoolExecutor(NSEED) as ex:
        J = sorted(ex.map(judge, pick), reverse=True)
    log('  after a follower: ' + ' '.join(f'{c}:{l0:+.1f}->{j:+.1f}' for j, l0, sd, c in J))
    if J[0][0] < 1.0:
        continue
    _, lead, sd, c = J[0]
    pre2 = cut(f'{D}/s{s}_t{sd}.txt', c, f'{D}/s{s}_cut.txt')
    # 2. follow the incumbent from there to the finish
    with ThreadPoolExecutor(NSEED) as ex:
        fins = list(ex.map(lambda q: tig(pre2, f'{D}/inc.csv', f'{D}/inc_track.txt', lead, 2800, f'{D}/s{s}_r{q}.txt', q, ['shadow=' + inc]),
                           range(NSEED)))
    ok = [(f, q) for q, f in enumerate(fins) if f > 0]
    log(f'  incumbent-tracker finishes: {fins}')
    if not ok:
        continue
    fT, q = min(ok)
    T = f'{D}/s{s}_r{q}.txt'
    # 3. chain with T as forced lineage
    tag = f'{name}_s{s}'
    out = sh(['python3', 'dschain.py', inc, str(c), tag, f'prefix0={pre2}', f'anc0={T}', 'sinks=auto', 'n=4', 'workers=4', 'beam=1000',
              'egain=0.002'])
    m = re.search(r'FINISH (\d+) \(incumbent (\d+)\) -> (\S+)', out)
    if not m:
        log(f'  chain {tag}: no finish')
        continue
    fin, run = int(m.group(1)), m.group(3)
    log(f'  chain {tag}: finish {fin} (T {fT}, incumbent {F}) ({time.time() - t0:.0f}s)')
    cand = [(fin, run), (fT, T)]
    for fc, rc in sorted(cand):
        if fc >= F:
            break
        sf = finish_of(rc)
        if sf is not None and sf < F:
            F = sf
            newb = f'kog_full_{F}.txt'
            subprocess.run(['cp', rc, newb])
            subprocess.run(['cp', rc, 'kog_full_best.txt'])
            log(f'NEW BEST {F} (server-checked) -> {newb}')
            inc = newb
            refresh()
            break
        log(f'  server check failed / not better: {sf}')
