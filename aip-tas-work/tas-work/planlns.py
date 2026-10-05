#!/usr/bin/env python3
"""Outer search over shot plans (seg plan=FILE): mutate the best plan (shift fire windows, move target points, drop or
add shots, free point-blank windows), run seg from PREFIX to GATE with the plan, keep the plan whose gate value
(race tick - selpost x energy) is lower.
usage: planlns.py PREFIX GATE PLAN DIR [minutes=240] [workers=4] [selpost=0.003] [opts="seg options"]"""
import os, random, re, subprocess, sys, threading, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'; MAP = 'AiP-Gores.map'
PREFIX, GATE, PLAN0, D = sys.argv[1:5]
os.makedirs(D, exist_ok=True)
args = dict(a.split('=', 1) for a in sys.argv[5:])
MIN = float(args.get('minutes', 240)); WORK = int(args.get('workers', 4)); SEL = float(args.get('selpost', 0.003))
PS2 = '1156,1285,1443,1793,2143,2465'
OPTS = args.get('opts', f'beam=10000 survevery=2 survive=30 ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 '
                        'rothook=1 quant=1 latpen=0.1 latdz=32 prefire=1 padtop=200').split()
lock = threading.Lock(); LOG = f'{D}/plan.log'

def log(m):
    with lock:
        open(LOG, 'a').write(time.strftime('%H:%M:%S ') + m + '\n')
    print(m, flush=True)

def parse(path):
    P = []
    for l in open(path):
        if l.startswith('#') or not l.strip(): continue
        w = l.split()
        if w[2] == 'free': P.append([int(w[0]), int(w[1]), 'free'])
        else: P.append([int(w[0]), int(w[1]), float(w[2]), float(w[3]), float(w[4]) if len(w) > 4 else 40.0])
    return sorted(P, key=lambda e: e[0])

def write(P, path):
    with open(path, 'w') as f:
        for e in sorted(P, key=lambda e: e[0]):
            f.write(f'{e[0]} {e[1]} free\n' if e[2] == 'free' else f'{e[0]} {e[1]} {e[2]:.0f} {e[3]:.0f} {e[4]:.0f}\n')

def run(P, tag):
    pf = f'{D}/{tag}.plan'; write(P, pf)
    cmd = [SEG, MAP, f'prefix={PREFIX}', f'gate={GATE}', 'horizon=150', 'threads=1', 'quiet=1', 'maxticks=400',
           f'out={D}/{tag}_', f'plan={pf}'] + OPTS
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    m = re.search(r'GATE rt (\d+) value ([-\d.]+) Ee (-?\d+)', res)
    if not m: return None
    return int(m.group(1)), int(m.group(3))

def mutate(P, rng):
    Q = [list(e) for e in P]
    k = rng.random()
    tgt = [i for i, e in enumerate(Q) if e[2] != 'free']
    if k < 0.35 and Q:                      # shift a window
        e = rng.choice(Q); s = rng.choice([-4, -3, -2, -1, 1, 2, 3, 4]); e[0] += s; e[1] += s
    elif k < 0.5 and Q:                     # narrow / widen a window
        e = rng.choice(Q); w = rng.choice([-2, -1, 1, 2])
        if e[1] - e[0] + 2 * w >= 0: e[0] -= w; e[1] += w
    elif k < 0.75 and tgt:                  # move a target point
        e = Q[rng.choice(tgt)]; e[2] += rng.uniform(-48, 48); e[3] += rng.uniform(-48, 48)
    elif k < 0.85 and len(Q) > 1:           # drop a shot
        Q.pop(rng.randrange(len(Q)))
    else:                                   # add a free point-blank window in a gap
        Q.sort(key=lambda e: e[0])
        bounds = [int(re.search(r'start: rt (\d+)', subprocess.run([SEG, MAP, f'prefix={PREFIX}', 'gate=finish', 'maxticks=0', 'quiet=1'], capture_output=True, text=True).stdout).group(1))]
        gaps = []
        prev = bounds[0]
        for e in Q:
            if e[0] - prev >= 30: gaps.append((prev + 25, e[0] - 25 if e[0] - 25 > prev + 25 else prev + 30))
            prev = e[1]
        if gaps:
            a, b = rng.choice(gaps); t = rng.randint(a, max(a, b)); Q.append([t, t + rng.randint(3, 10), 'free'])
    return sorted(Q, key=lambda e: e[0])

best_plan = parse(PLAN0)
r = run(best_plan, 'init')
best = (r[0] - SEL * r[1], r[0], r[1]) if r else (1e9, None, None)
log(f'init plan: gate rt {best[1]} Ee {best[2]}')
write(best_plan, f'{D}/best.plan')
stop = time.time() + MIN * 60
def worker(wid):
    global best, best_plan
    rng = random.Random(wid * 9973 + int(time.time()))
    it = 0
    while time.time() < stop:
        it += 1
        with lock:
            P = mutate(best_plan, rng)
        r = run(P, f'w{wid}')
        if not r:
            log(f'w{wid} it{it}: NOGATE'); continue
        v = r[0] - SEL * r[1]
        with lock:
            if v < best[0] - 1e-6:
                best = (v, r[0], r[1]); best_plan = P
                write(P, f'{D}/best.plan'); os.replace(f'{D}/w{wid}_0.txt', f'{D}/best_{r[0]}.txt')
                log(f'w{wid} it{it}: rt {r[0]} Ee {r[1]}  ** NEW BEST ** plan {P}')
            else:
                log(f'w{wid} it{it}: rt {r[0]} Ee {r[1]} (best {best[1]}/{best[2]})')
with ThreadPoolExecutor(WORK) as ex:
    list(ex.map(worker, range(WORK)))
