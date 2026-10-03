#!/usr/bin/env python3
"""Iterated re-search (large neighbourhood search) for corridor 1 with the energy beam.

Repeatedly cuts the current best run (inputs from spawn) at a random race tick, re-searches the rest
with `pre` (post-start mode, gate x > STOPX) under a random parameter variant, verifies the result
by replay and keeps it when it reaches the gate in fewer race ticks.

usage: lns.py BEST.txt [minutes] [workers]      (state in runs/lns/)
"""
import math
import os
import random
import re
import subprocess
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
PRE = '../ddnet/build-sim/pre'
LAB = '../ddnet/build-sim/lab'
MAP = 'AiP-Gores.map'
STOPX = 8800
D = os.environ.get('LNS_DIR', 'runs/lns')
ROT = int(os.environ.get('LNS_ROT', '0'))
MINCUT = int(os.environ.get('LNS_MINCUT', '0'))
LA = int(os.environ.get('LNS_LA', '0'))
os.makedirs(D, exist_ok=True)


def race_ticks(inputs_file):
    """per input index: race tick after that input (-1 before the start), and the gate race tick"""
    out = subprocess.run([LAB, MAP, 'replay ' + inputs_file], capture_output=True, text=True).stdout
    rts, gate = [], None
    for line in out.splitlines():
        m = re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+)', line)
        if not m or not line.startswith('in '):
            continue
        rt, x = int(m.group(1)), float(m.group(3))
        rts.append(rt)
        if gate is None and rt >= 0 and x > STOPX:
            gate = rt
    return rts, gate


lock = threading.Lock()
best_file = os.path.join(D, 'best.txt')
log_file = os.path.join(D, 'lns.log')


def log(msg):
    with lock:
        with open(log_file, 'a') as f:
            f.write(time.strftime('%H:%M:%S ') + msg + '\n')
    print(msg, flush=True)


def variant(rng):
    v = {
        'postbeam': rng.choice([20000, 30000, 30000, 45000]),
        'postdir1': 1,
        'angles': rng.choice([48, 64, 64, 96]),
        'pjc': rng.choice([100, 130, 130]),
        'pgc': rng.choice([120, 160, 160]),
    }
    r = rng.random()
    if r < 0.75:
        # time model with the current speed over the next H px (best so far)
        v['trref'] = rng.choice(['teero_track.txt', 'teero_track.txt', 'own_track.txt'])
        v['hnow'] = rng.choice([100, 150, 200, 300, 300, 450])
    else:
        v['lambda'] = rng.choice([0.03, 0.05, 0.07, 0.09])
        v['vref'] = rng.choice([25, 30])
    if ROT:
        v['rothook'] = 1
    if LA:
        v['hookla'] = LA
        if 'trref' not in v:  # the lookahead only works with the time model
            v['trref'] = 'teero_track.txt'
            v['hnow'] = rng.choice([150, 300])
    return v


job_counter = [0]


def job(seed):
    rng = random.Random(seed)
    with lock:
        job_counter[0] += 1
        k = job_counter[0]
        best = open(best_file).read().splitlines()
    rts, best_rt = race_ticks(best_file)
    # cut at a random race tick (inputs after the start), biased to the first two thirds
    started = [i for i, r in enumerate(rts) if r >= 0]
    if not started:
        return
    first = started[0]
    cut_rt = max(MINCUT, int(rng.triangular(0, best_rt - 15, best_rt * 0.35)))
    idx = next(i for i in started if rts[i] >= cut_rt) + 1
    pf = os.path.join(D, f'j{k}_pf.txt')
    with open(pf, 'w') as f:
        f.write('\n'.join(best[:idx]) + '\n')
    v = variant(rng)
    args = [PRE, MAP, f'prefix={pf}', 'threads=1', f'gatex={STOPX}', 'gatelambda=0', 'top=1',
            f'maxticks={len(best) - idx + 40}', f'out={D}/j{k}_']
    args += [f'{a}={b}' for a, b in v.items()]
    t0 = time.time()
    res = subprocess.run(args, capture_output=True, text=True)
    m = re.search(r'^CROSS 0 rank (-?[\d.]+)', res.stdout, re.M)
    out = f'{D}/j{k}_0.txt'
    if not m or not os.path.exists(out):
        log(f'job {k} cut rt {cut_rt} {v}: no gate ({time.time() - t0:.0f}s)')
        return
    _, rt = race_ticks(out)
    msg = f'job {k} cut rt {cut_rt} {v}: rt {rt} (best {best_rt}) ({time.time() - t0:.0f}s)'
    with lock:
        _, cur = race_ticks(best_file)
        if rt is not None and rt < cur:
            os.replace(out, best_file)
            with open(os.path.join(D, f'best_{rt}.txt'), 'w') as f:
                f.write(open(best_file).read())
            msg += '  ** NEW BEST'
        else:
            for suffix in ('0.txt',):
                try:
                    os.remove(f'{D}/j{k}_{suffix}')
                except OSError:
                    pass
        try:
            os.remove(pf)
        except OSError:
            pass
    log(msg)


def main():
    src = sys.argv[1]
    minutes = float(sys.argv[2]) if len(sys.argv) > 2 else 60
    workers = int(sys.argv[3]) if len(sys.argv) > 3 else 4
    if not os.path.exists(best_file):
        with open(best_file, 'w') as f:
            f.write(open(src).read())
    _, rt = race_ticks(best_file)
    log(f'start: best {rt} from {src}, {minutes} min, {workers} workers')
    end = time.time() + minutes * 60
    seed = int(time.time())
    with ThreadPoolExecutor(workers) as ex:
        running = set()
        while time.time() < end or running:
            while time.time() < end and len(running) < workers:
                seed += 1
                running.add(ex.submit(job, seed))
            done = {f for f in running if f.done()}
            for f in done:
                try:
                    f.result()
                except Exception as e:  # keep going
                    log(f'job error: {e!r}')
            running -= done
            time.sleep(2)
    _, rt = race_ticks(best_file)
    log(f'end: best {rt}')


if __name__ == '__main__':
    main()
