#!/usr/bin/env python3
"""Anchored LNS on the pre-grenade section with seg (lns2.py + anchor=best run, so a job never returns a worse run).

Cut the best run (spawn -> grenade pickup) at a random race tick, re-search to the pickup (gate=grenade) with a random
seg variant (optionally a jittered beam), keep the result when it is better by the sub-tick pickup time:
  t* = race tick at which the tee's centre enters the 48 px pickup circle (interpolated inside the tick); the grenade
  is taken one tick after the first tick that ends inside the circle, so pickup race tick = floor(t*) + 2.
Sub-tick gains are kept so that small improvements can add up to whole ticks. A post-pickup climb check (reach the
reference's rt-1005 point within 80 ticks, movement only) must pass before a run is accepted.

usage: lns2.py BEST.txt DIR [minutes=120] [workers=4] [cutmin=0] [cutmax=] [ref=own_track.txt] [late=0.5]
"""
import math, os, random, re, subprocess, sys, threading, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'; LAB = '../ddnet/build-sim/lab'; MAP = 'AiP-Gores.map'
GREN = (170 * 32 + 16, 77 * 32 + 16); PICK_R = 48.0
D = sys.argv[2]; os.makedirs(D, exist_ok=True)
args = dict(a.split('=', 1) for a in sys.argv[3:])
MIN = float(args.get('minutes', 120)); WORK = int(args.get('workers', 4)); CUTMIN = int(args.get('cutmin', 0))
CUTMAX = int(args['cutmax']) if 'cutmax' in args else None
REF = args.get('reftrack', args.get('ref', 'own_track.txt')); LATE = float(args.get('late', 0.3))
BEAMS = [int(b) for b in args.get('beams', '3000,5000,5000,8000').split(',')]
GHOSTE = [float(b) for b in args.get('ghoste', '0.01,0.02,0.02,0.03').split(',')]
LATSTRONG = float(args.get('latstrong', 0.0))
best_file = f'{D}/best.txt'; log_file = f'{D}/lns.log'; lock = threading.Lock()
SINKS = '315,550,725,900'


def log(m):
    with lock:
        open(log_file, 'a').write(time.strftime('%H:%M:%S ') + m + '\n')
    print(m, flush=True)


def replay(path):
    """per input: (rt, x, y, gren) from a lab replay"""
    out = subprocess.run([LAB, MAP, 'replay ' + path], capture_output=True, text=True).stdout
    R = []
    for l in out.splitlines():
        m = re.search(r'rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*frz (\d) gren (\d)', l)
        if m and l.startswith('in '):
            R.append((int(m.group(1)), float(m.group(2)), float(m.group(3)), int(m.group(5)), int(m.group(4))))
    return R


def score(path):
    """(t*, pickup rt, end rt list) or None: t* = sub-tick entry into the pickup circle"""
    R = replay(path)
    k = next((i for i, r in enumerate(R) if r[3]), None)
    if k is None or k < 2 or any(r[4] for r in R[:k + 1]):
        return None
    # the grenade is taken in the tick after the first tick that ends inside the circle (index k-1)
    a, b = R[k - 2], R[k - 1]
    da = math.hypot(a[1] - GREN[0], a[2] - GREN[1]); db = math.hypot(b[1] - GREN[0], b[2] - GREN[1])
    f = 1.0 if da <= db else min(1.0, max(0.0, (da - PICK_R) / (da - db)))
    return (a[0] + f, R[k][0], [r[0] for r in R])


def seg_gate_rt(res):
    m = re.search(r'GATE rt (\d+)', res)
    return int(m.group(1)) if m else None


def variant(rng):
    v = {'beam': rng.choice(BEAMS), 'angles': rng.choice([64, 128, 128]), 'rothook': 1, 'quant': 1}
    if v['angles'] == 128:
        v['hookdedup'] = 0
    g = rng.random()
    if g < 0.7:
        v.update(ghost=1, hnow=rng.choice([300, 600, 600, 1000]), ghoste=rng.choice(GHOSTE))
    else:
        v.update(ghost=3, hnow=1000, ghoste=0.02, sinks=SINKS, ghostsink=1500)
    if rng.random() < 0.3:
        v['dirmode'] = 'tan'
    if rng.random() < 0.4:
        v['cpos'] = 8; v['cvel'] = 1
    if rng.random() < 0.3:
        v['pjc'] = 250; v['pgc'] = 250
    r = rng.random()
    if r < LATSTRONG:
        # stay on the anchor's own line (reference = the current best): deviations keep its polished continuation
        v['latpen'] = rng.choice([0.05, 0.1, 0.1, 0.2, 0.3]); v['latdz'] = rng.choice([4, 8, 8, 16])
    elif r < LATSTRONG + 0.2:
        v['latpen'] = rng.choice([0.03, 0.05, 0.05]); v['latdz'] = rng.choice([24, 32])
    if rng.random() < 0.5:
        v['jitter'] = rng.choice([0.3, 0.6, 1.0, 1.5]); v['seed'] = rng.randrange(1 << 30)
    if rng.random() < 0.5:
        v['ancinj'] = rng.choice([1, 1, 2])
    return v


def viable(path, k):
    # climb check: with the grenade, reach the upper shaft (box) within 80 ticks, movement only
    res = subprocess.run([SEG, MAP, f'prefix={path}', f'ref={REF}', 'gate=box:5150,5420,1900,2200', 'fire=0', 'horizon=150',
                          'beam=20000', 'threads=1', 'maxticks=80', 'quiet=1', 'survevery=0', 'rothook=1', 'quant=1', 'ghost=1',
                          'hnow=300', 'ghoste=0.02', 'angles=128', 'hookdedup=0', f'out={D}/v{k}_'],
                         capture_output=True, text=True).stdout
    for f in (f'{D}/v{k}_0.txt', f'{D}/v{k}_c.txt'):
        if os.path.exists(f): os.remove(f)
    return seg_gate_rt(res) is not None


cnt = [0]


def job(seed):
    rng = random.Random(seed)
    with lock:
        cnt[0] += 1; k = cnt[0]
        best = open(best_file).read().splitlines()
        cur = float(open(f'{D}/best_t').read())
    sc = score(best_file)
    rts = sc[2]
    end_rt = sc[1]
    hi = min(end_rt - 12, CUTMAX if CUTMAX is not None else end_rt)
    if rng.random() < LATE:
        cut = int(rng.uniform(max(CUTMIN, hi - 250), hi))
    else:
        cut = int(rng.uniform(CUTMIN, hi))
    idx = next(i for i, r in enumerate(rts) if r >= cut and i > 60) + 1
    pf = f'{D}/j{k}_pf.txt'
    open(pf, 'w').write('\n'.join(best[:idx]) + '\n')
    open(f'{best_file}.job{k}', 'w').write('\n'.join(best) + '\n')
    v = variant(rng)
    cmd = [SEG, MAP, f'prefix={pf}', f'anchor={best_file}.job{k}', 'gatet=1', 'gatejump=1', f'ref={REF}', 'gate=grenade', 'horizon=150', 'threads=1',
           f'maxticks={2 * (end_rt - cut) + 60}', 'quiet=1', 'survevery=10', f'out={D}/j{k}_'] + [f'{a}={b}' for a, b in v.items()]
    t0 = time.time()
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    out = f'{D}/j{k}_0.txt'
    msg = f'job {k} cut {cut} {v}: '
    for f in (pf, f'{D}/j{k}_c.txt', f'{best_file}.job{k}'):
        if os.path.exists(f): os.remove(f)
    if seg_gate_rt(res) is None or not os.path.exists(out):
        log(msg + f'no gate ({time.time() - t0:.0f}s)'); return
    s = score(out)
    if s is None:
        log(msg + 'invalid result'); os.remove(out); return
    msg += f'pickup {s[1]} t* {s[0]:.3f} ({time.time() - t0:.0f}s)'
    if s[0] < cur - 1e-3:
        if not viable(out, k):
            os.makedirs(f'{D}/noclimb', exist_ok=True)
            os.replace(out, f'{D}/noclimb/j{k}_{s[0]:.3f}.txt')
            log(msg + '  [post-pickup climb check FAILED]'); return
        with lock:
            cur = float(open(f'{D}/best_t').read())
            if s[0] < cur - 1e-3:
                os.replace(out, best_file)
                open(f'{D}/best_t', 'w').write(f'{s[0]:.4f}')
                open(f'{D}/best_{s[1]}_{s[0]:.3f}.txt', 'w').write(open(best_file).read())
                msg += f'  ** NEW BEST (was t* {cur:.3f})'
                if args.get('reftrack'):
                    # keep the reference line = the current best (lag is measured against the anchor itself)
                    subprocess.run(['./mktrack.py', best_file, f'{D}/track_tmp.txt'], capture_output=True)
                    os.replace(f'{D}/track_tmp.txt', args['reftrack'])
    if os.path.exists(out):
        os.remove(out)
    log(msg)


def main():
    if not os.path.exists(best_file):
        open(best_file, 'w').write(open(sys.argv[1]).read())
        s = score(best_file)
        open(f'{D}/best_t', 'w').write(f'{s[0]:.4f}')
    log(f'start: best t* {open(f"{D}/best_t").read()} from {sys.argv[1]}, {MIN} min, {WORK} workers, args {args}')
    end = time.time() + MIN * 60; seed = int(time.time() * 1000) % (1 << 30)
    with ThreadPoolExecutor(WORK) as ex:
        running = set()
        while time.time() < end or running:
            while time.time() < end and len(running) < WORK:
                seed += 1; running.add(ex.submit(job, seed))
            done = {f for f in running if f.done()}
            for f in done:
                try: f.result()
                except Exception as e: log(f'job error {e!r}')
            running -= done
            time.sleep(2)
    log(f'end: best t* {open(f"{D}/best_t").read()}')


main()
