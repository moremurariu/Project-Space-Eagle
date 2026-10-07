#!/usr/bin/env python3
"""LNS on a complete run with `tas`: cut the current best at a random input index (>= cutmin), search from there to
the finish with randomized tas settings, keep the result if it finishes earlier. Several workers share the best.

usage: lnstas.py BEST_FILE NAME [workers=4] [cutmin=1150] [hours=6]   (state in runs/lnstas/NAME/)
"""
import os
import random
import re
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
TAS = '../ddnet/build-sim/tas'
MAP = 'AiP-Gores.map'
args = dict(a.split('=', 1) for a in sys.argv[3:])
WORKERS = int(args.get('workers', 4))
CUTMIN = int(args.get('cutmin', 1150))
HOURS = float(args.get('hours', 6))
D = f'runs/lnstas/{sys.argv[2]}'
os.makedirs(D, exist_ok=True)
LOG = f'{D}/lns.log'
lock = threading.Lock()


def log(msg):
    with lock:
        with open(LOG, 'a') as f:
            f.write(time.strftime('%H:%M:%S ') + msg + '\n')
    print(msg, flush=True)


def finish_ticks(path):
    out = subprocess.run([TAS, MAP, 'replay', path], capture_output=True, text=True).stdout
    m = re.search(r'finish.*?(\d+) ticks', out)
    return int(m.group(1)) if m else None


best_path = f'{D}/best.txt'
if not os.path.exists(best_path):
    with open(best_path, 'w') as f:
        f.write(open(sys.argv[1]).read())
best = [int(args['bestticks'])] if 'bestticks' in args else [None]
best_lines = [open(best_path).read().splitlines()]


def worker(w):
    t_end = time.time() + HOURS * 3600
    it = 0
    while time.time() < t_end:
        it += 1
        with lock:
            lines = list(best_lines[0])
            cur = best[0]
        n = len(lines)
        hi = n - 120
        if hi <= CUTMIN:
            return
        # bias towards later cuts (cheaper), but cover the whole range
        c = int(CUTMIN + (hi - CUTMIN) * random.random() ** 0.7)
        pre = f'{D}/w{w}_pre.txt'
        with open(pre, 'w') as f:
            f.write('\n'.join(lines[:c]) + '\n')
        params = {
            'beam': random.choice([1000, 1000, 1500, 2000]),
            'kcredit': random.choice([0.8, 1, 1, 1.25, 1.5]),
            'kready': random.choice([8, 10, 10, 12]),
            'energyshare': random.choice([0.3, 0.4, 0.4, 0.5]),
            'jumpenergy': random.choice([150, 200, 200, 250]),
            'survive': random.choice([15, 15, 20]),
            'vref': random.choice([25, 25, 30]),
        }
        out = f'{D}/w{w}_out.txt'
        cmd = [TAS, MAP, 'search', 'firelook=1', 'stencil16=3', 'repeat=1', f'prefix={pre}', f'maxticks={n - c + 200}', 'threads=1',
               f'out={out}'] + [f'{k}={v}' for k, v in params.items()]
        t0 = time.time()
        res = subprocess.run(cmd, capture_output=True, text=True).stdout
        m = re.search(r'FINISH (\d+) ticks', res)
        r = int(m.group(1)) if m else None
        ps = ' '.join(f'{k}={v}' for k, v in params.items())
        with lock:
            improved = r is not None and (best[0] is None or r < best[0])
            if improved:
                best[0] = r
                best_lines[0] = open(out).read().splitlines()
                with open(best_path, 'w') as f:
                    f.write('\n'.join(best_lines[0]) + '\n')
                with open(f'{D}/best_{r}.txt', 'w') as f:
                    f.write('\n'.join(best_lines[0]) + '\n')
        log(f'w{w} it{it} cut {c}/{n} [{ps}] -> {r} (best {best[0]}){" NEW BEST" if improved else ""} ({time.time() - t0:.0f}s)')


ths = [threading.Thread(target=worker, args=(w,)) for w in range(WORKERS)]
for t in ths:
    t.start()
for t in ths:
    t.join()
log(f'done, best {best[0]}')
