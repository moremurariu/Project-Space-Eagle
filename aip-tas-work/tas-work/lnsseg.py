#!/usr/bin/env python3
"""LNS on the pre-grenade section with seg: cut the best run (spawn -> grenade pickup) at a random race tick,
re-search to the pickup (gate=grenade) with a random variant, keep it when the pickup comes earlier (verified by
replay in lab: pickup race tick = the tick the tee gets the grenade).

usage: lnsseg.py BEST.txt DIR [minutes=120] [workers=4] [cutmin=250]
"""
import os, random, re, subprocess, sys, threading, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'; LAB = '../ddnet/build-sim/lab'; MAP = 'AiP-Gores.map'
D = sys.argv[2]; os.makedirs(D, exist_ok=True)
args = dict(a.split('=', 1) for a in sys.argv[3:])
MIN = float(args.get('minutes', 120)); WORK = int(args.get('workers', 4)); CUTMIN = int(args.get('cutmin', 250))
best_file = f'{D}/best.txt'; log_file = f'{D}/lns.log'; lock = threading.Lock()
SINKS = '315,550,725,900'

def log(m):
    with lock:
        open(log_file, 'a').write(time.strftime('%H:%M:%S ') + m + '\n')
    print(m, flush=True)

def pickup_rt(path):
    """(race tick per input index, pickup race tick) from a lab replay (weapon appears in the lab line as 'gren')"""
    out = subprocess.run([LAB, MAP, 'replay ' + path], capture_output=True, text=True).stdout
    rts = []
    for l in out.splitlines():
        m = re.search(r'rt=(-?\d+) ', l)
        if m and l.startswith('in '):
            rts.append(int(m.group(1)))
    return rts

def seg_gate_rt(res):
    m = re.search(r'GATE rt (\d+)', res)
    return int(m.group(1)) if m else None

def variant(rng):
    v = {'beam': rng.choice([20000, 20000, 30000]), 'angles': rng.choice([64, 128, 128]), 'rothook': 1, 'quant': 1}
    if v['angles'] == 128:
        v['hookdedup'] = 0
    g = rng.random()
    if g < 0.7:
        v.update(ghost=1, hnow=rng.choice([300, 600, 600, 1000]), ghoste=rng.choice([0.01, 0.02, 0.02, 0.03]))
    else:
        v.update(ghost=3, hnow=1000, ghoste=0.02, sinks=SINKS, ghostsink=1500)
    if rng.random() < 0.4:
        v['dirmode'] = 'tan'
    if rng.random() < 0.4:
        v['cpos'] = 8; v['cvel'] = 1
    if rng.random() < 0.3:
        v['pjc'] = 250; v['pgc'] = 250
    return v

cnt = [0]
def job(seed):
    rng = random.Random(seed)
    with lock:
        cnt[0] += 1; k = cnt[0]
        best = open(best_file).read().splitlines()
    rts = pickup_rt(best_file)
    end_rt = max(rts)
    cut = int(rng.uniform(CUTMIN, end_rt - 20))
    idx = next(i for i, r in enumerate(rts) if r >= cut) + 1
    pf = f'{D}/j{k}_pf.txt'
    open(pf, 'w').write('\n'.join(best[:idx]) + '\n')
    v = variant(rng)
    cmd = [SEG, MAP, f'prefix={pf}', 'gate=grenade', 'horizon=150', 'threads=1', f'maxticks={2 * (end_rt - cut) + 60}',
           'quiet=1', 'survevery=10', f'out={D}/j{k}_'] + [f'{a}={b}' for a, b in v.items()]
    t0 = time.time()
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    rt = seg_gate_rt(res)
    out = f'{D}/j{k}_0.txt'
    msg = f'job {k} cut {cut} {v}: '
    if rt is None or not os.path.exists(out):
        log(msg + f'no gate ({time.time() - t0:.0f}s)'); return
    msg += f'pickup {rt} ({time.time() - t0:.0f}s)'
    with lock:
        cur = int(open(f'{D}/best_rt').read())
        if rt < cur:
            os.replace(out, best_file)
            open(f'{D}/best_rt', 'w').write(str(rt))
            open(f'{D}/best_{rt}.txt', 'w').write(open(best_file).read())
            msg += f'  ** NEW BEST (was {cur})'
        else:
            os.remove(out)
    for f in (pf, f'{D}/j{k}_c.txt'):
        if os.path.exists(f): os.remove(f)
    log(msg)

def main():
    if not os.path.exists(best_file):
        open(best_file, 'w').write(open(sys.argv[1]).read())
        open(f'{D}/best_rt', 'w').write(args.get('rt', '99999'))
    log(f'start: best {open(f"{D}/best_rt").read()} from {sys.argv[1]}, {MIN} min, {WORK} workers')
    end = time.time() + MIN * 60; seed = int(time.time())
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
            time.sleep(5)
    log(f'end: best {open(f"{D}/best_rt").read()}')

main()
