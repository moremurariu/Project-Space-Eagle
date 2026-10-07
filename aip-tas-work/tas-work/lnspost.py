#!/usr/bin/env python3
"""LNS on a complete run (post-grenade part) with seg: cut the best run at a random race tick >= cutmin, re-search to
the finish (gate=finish) with a random variant, keep it when the finish comes earlier (seg's finish tick is exact).
usage: lnspost.py BEST.txt DIR [minutes=600] [workers=4] [cutmin=1030] [cutmax=]"""
import os, random, re, subprocess, sys, threading, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'; MAP = 'AiP-Gores.map'
D = sys.argv[2]; os.makedirs(D, exist_ok=True)
args = dict(a.split('=', 1) for a in sys.argv[3:])
MIN = float(args.get('minutes', 600)); WORK = int(args.get('workers', 4)); CUTMIN = int(args.get('cutmin', 1030))
best_file = f'{D}/best.txt'; log_file = f'{D}/lns.log'; lock = threading.Lock()
PS2 = '1156,1285,1443,1793,2143,2465'
if not os.path.exists(best_file):
    open(best_file, 'w').write(open(sys.argv[1]).read())

def log(m):
    with lock:
        open(log_file, 'a').write(time.strftime('%H:%M:%S ') + m + '\n')
    print(m, flush=True)

def finish_rt(path):
    res = subprocess.run([SEG, MAP, f'prefix={path}', 'gate=finish', 'maxticks=0', 'quiet=1'], capture_output=True, text=True).stdout
    m = re.search(r'start: rt (-?\d+)', res)
    return int(m.group(1)) if m else None

def variant(rng):
    v = {'beam': rng.choice([8000, 10000, 10000, 15000]), 'survevery': 2, 'survive': rng.choice([20, 30, 30, 40]),
         'rothook': 1, 'quant': 1, 'kcredit': rng.choice([0, 1, 2, 2]), 'kready': 10}
    g = rng.random()
    if g < 0.6:
        v.update(ghost=3, hnow=rng.choice([600, 1000, 1000]), ghoste=rng.choice([0.0, 0.01, 0.02, 0.02]), sinks=PS2, ghostsink=1500)
        if rng.random() < 0.3:
            v['brake'] = 2
    else:
        v.update(ghost=1, hnow=rng.choice([300, 600]), ghoste=rng.choice([0.01, 0.02]))
    v['latpen'] = rng.choice([0.05, 0.1, 0.1, 0.2]); v['latdz'] = rng.choice([24, 32, 32, 48])
    if rng.random() < 0.5:
        v['quota'] = rng.choice([4, 6, 10])
    if rng.random() < 0.25:
        v.update(prefire=1, padaims=48, padtop=300, padrange=30)
    if rng.random() < 0.2:
        v.update(angles=128, hookdedup=0)
    return v

stop = time.time() + MIN * 60
def worker(wid):
    rng = random.Random(wid * 7919 + int(time.time()))
    it = 0
    while time.time() < stop:
        it += 1
        with lock:
            best = open(best_file).read().splitlines()
        F = len(best) - 68  # inputs end at the finish tick
        hi = int(args.get('cutmax', F - 40))
        c = rng.randint(CUTMIN, max(CUTMIN, hi))
        pre = f'{D}/w{wid}_pre.txt'
        open(pre, 'w').write('\n'.join(best[:c + 68]) + '\n')
        v = variant(rng)
        out = f'{D}/w{wid}_'
        cmd = [SEG, MAP, f'prefix={pre}', 'gate=finish', 'horizon=150', 'threads=1', 'quiet=1', f'maxticks={F - c + 80}',
               f'out={out}'] + [f'{k}={x}' for k, x in v.items()]
        t0 = time.time()
        res = subprocess.run(cmd, capture_output=True, text=True).stdout
        m = re.search(r'GATE rt (\d+)', res)
        f = int(m.group(1)) if m else None
        vs = ' '.join(f'{k}={x}' for k, x in v.items())
        if f is None:
            log(f'w{wid} it{it} cut {c}: NOGATE ({time.time()-t0:.0f}s) [{vs}]')
            continue
        with lock:
            Fb = len(open(best_file).read().splitlines()) - 68
            if f < Fb:
                new = open(out + '0.txt').read()
                open(best_file, 'w').write(new)
                open(f'{D}/best_{f}.txt', 'w').write(new)
                msg = f'w{wid} it{it} cut {c}: finish {f} < {Fb}  ** NEW BEST ** ({time.time()-t0:.0f}s) [{vs}]'
            else:
                msg = f'w{wid} it{it} cut {c}: finish {f} (best {Fb}) ({time.time()-t0:.0f}s) [{vs}]'
        log(msg)

with ThreadPoolExecutor(WORK) as ex:
    list(ex.map(worker, range(WORK)))
