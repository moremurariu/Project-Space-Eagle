#!/usr/bin/env python3
"""Same-budget LNS benchmark for the branches' TAS finders.
usage: lnsbench.py TOOL START DIR [minutes=12] [workers=4] [cutmin=] [seed=1]
TOOL (each with the parameter variants of its own branch's LNS driver):
  ddsearch    faraday   CFast beam search, incumbent kept            (tools/lns.py variants)      gate: pickup
  seg-cannon  cannon    seg on CTasGame(+FastCopy), anchor=best      (lns3.py variants)           gate: pickup
              (anchor= exists only in cannon's seg; on the merged build use segf-cray, whose inc= is the same idea)
  segf-cray   cray      seg on CFastG, inc=best                      (lns3.py variants)           gate: pickup
  segf-dav    davinci   seg on CFastG, inc=best                      (lnsinc.py variants)         gate: finish
  segfx-cray  cray      segf + survx (cray's late-cut recipe)        (lnsinc.py + survx variants) gate: finish
  xds-dav     davinci   x_ds deferred-shot search, inc=best          (dsloop/dschain params)      gate: finish
Every improvement is re-checked on the real server code (TasReplay) before it is accepted. Logs to DIR/lns.log.
"""
import os, random, re, subprocess, sys, threading, time

# Run from anywhere; W = tas-work (map, mapk.py, teero_track.txt). By default every finder runs on the merged build
# (ddnet/build-sim). The branch comparison in BENCHMARKS.md used one build per branch: set BENCH_TREES to
# "far=DIR,cannon=DIR,dav=DIR,cray=DIR" (build directories of the branches' own trees) to reproduce it.
W = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_merged = os.path.join(W, '..', 'ddnet', 'build-sim')
B = {t: _merged for t in ('far', 'cannon', 'dav', 'cray')}
for _kv in filter(None, os.environ.get('BENCH_TREES', '').split(',')):
    _k, _v = _kv.split('=', 1)
    B[_k] = _v
MAP = f'{W}/AiP-Gores.map'
TOOL, START, D = sys.argv[1], sys.argv[2], sys.argv[3]
kw = dict(a.split('=', 1) for a in sys.argv[4:])
MIN = float(kw.get('minutes', 12)); NW = int(kw.get('workers', 4))
PRE = TOOL in ('ddsearch', 'seg-cannon', 'segf-cray')
CUTMIN = int(kw.get('cutmin', 300 if PRE else 1100))
os.makedirs(D, exist_ok=True)
lock = threading.Lock()
T_END = time.time() + MIN * 60


def log(m):
    with lock:
        with open(f'{D}/lns.log', 'a') as f:
            f.write(time.strftime('%H:%M:%S ') + m + '\n')
    print(m, flush=True)


def server(path):
    """(pickup rt, finish rt) on the real server code, or None if frozen / double start / failed"""
    env = dict(os.environ, TAS_MAP=MAP, TAS_INPUTS=os.path.abspath(path))
    out = subprocess.run([f'{B["dav"]}/testrunner', '--gtest_filter=TasReplay.Run'], cwd=B['dav'], env=env,
                         capture_output=True, text=True).stdout
    if 'frozen ticks 0' not in out or 'DOUBLE' in out or '[       OK ]' not in out:
        return None
    p = re.search(r'-> race tick (\d+)', out)
    f = re.search(r'finish tick (\d+) -> (\d+) ticks', out)
    return (int(p.group(1)) if p else None, int(f.group(2)) if f else None)


def metric(path):
    r = server(path)
    if r is None:
        return None
    return r[0] if PRE else r[1]


best_file = f'{D}/best.txt'
if not os.path.exists(best_file):
    open(best_file, 'w').write(open(START).read())
state = {'best': metric(best_file), 'track_for': None, 'track': None, 'jobs': 0, 'gains': []}
log(f'start {TOOL}: best {state["best"]} ({MIN} min, {NW} workers, cutmin {CUTMIN})')


def own_track(text):
    """reference track of the current best ("k x y", k = rt + 3), regenerated when the best changes"""
    with lock:
        if state['track_for'] == text:
            return state['track']
        n = len(state['gains'])
        run = f'{D}/ref_{n}.txt'; tr = f'{D}/ref_{n}_track.txt'
        open(run, 'w').write(text)
        subprocess.run([f'{B["dav"]}/segf', MAP, f'prefix={run}', 'gate=grenade' if PRE else 'gate=finish'],
                       env=dict(os.environ, SEG_TRACK=tr), capture_output=True, text=True)
        state['track_for'] = text; state['track'] = tr
        if not PRE:  # Teero's speed minima mapped onto this run (davinci mapk.py), for the ghost=3 variants
            state['sinks'] = subprocess.run(['python3', 'mapk.py', tr, TSINKS], cwd=W, capture_output=True,
                                            text=True).stdout.strip()
        return tr


# ------------------------------------------------------------------ variants (copied from each branch's driver)
def v_ddsearch(rng):  # faraday tools/lns.py
    v = {'beam': rng.choice([4000, 8000, 8000, 16000, 30000]), 'hnow': rng.choice([300, 600, 600, 1000]),
         'ge': rng.choice([0, 0.01, 0.02, 0.02, 0.04]), 'angles': rng.choice([64, 64, 128]), 'rothook': rng.choice([0, 1, 1]),
         'survive': rng.choice([6, 8, 8, 12]), 'cellpos': rng.choice([4, 8, 8, 12]), 'cellvel': rng.choice([0.5, 1, 1, 2]),
         'seed': rng.randrange(1, 1 << 30)}
    if rng.random() < 0.4:
        v['lp'] = rng.choice([0.03, 0.05, 0.1]); v['latdz'] = rng.choice([16, 24, 32])
    if rng.random() < 0.4:
        v['jitter'] = rng.choice([0.3, 0.7, 1.5])
    return v


def v_lns3(rng):  # cannon lns3.py (pre-grenade seg)
    v = {'beam': rng.choice([3000, 5000, 5000, 8000]), 'angles': rng.choice([64, 128, 128]), 'rothook': 1, 'quant': 1}
    if v['angles'] == 128:
        v['hookdedup'] = 0
    v.update(ghost=1, hnow=rng.choice([300, 600, 600, 1000]), ghoste=rng.choice([0.01, 0.02, 0.02, 0.03]))
    if rng.random() < 0.3:
        v['dirmode'] = 'tan'
    if rng.random() < 0.4:
        v['cpos'] = 8; v['cvel'] = 1
    if rng.random() < 0.3:
        v['pjc'] = 250; v['pgc'] = 250
    if rng.random() < 0.2:
        v['latpen'] = rng.choice([0.03, 0.05, 0.05]); v['latdz'] = rng.choice([24, 32])
    if rng.random() < 0.5:
        v['jitter'] = rng.choice([0.3, 0.6, 1.0, 1.5]); v['seed'] = rng.randrange(1 << 30)
    return v


def v_lnsinc(rng):  # davinci lnsinc.py (own reference)
    v = {'beam': rng.choice([8000, 10000, 15000, 20000]), 'survevery': 2, 'survive': rng.choice([20, 30, 30, 40]),
         'rothook': 1, 'quant': 1, 'kcredit': rng.choice([0, 1, 2, 2]), 'kready': 10}
    if rng.random() < 0.6:
        v.update(ghost=3, hnow=rng.choice([600, 1000, 1000]), ghoste=rng.choice([0.0, 0.01, 0.02, 0.02]), ghostsink=1500,
                 vcap=rng.choice([1.25, 1.5, 2.0]), sinks=state['sinks'])
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
    if rng.random() < 0.7:  # lnsinc retro=1
        v['retro'] = rng.choice([1, 3, 3])
        if rng.random() < 0.6:
            v.update(loadres=rng.choice([0.2, 0.4]), quota=30)
    if rng.random() < 0.5:
        v.update(jitter=1, seed=rng.randint(1, 10**6))
    return v


def v_lnsinc_survx(rng):  # cray: lnsinc + the late-cut survx recipe (segx survx=300 jitter=1 seed=N)
    v = v_lnsinc(rng)
    if rng.random() < 0.6:
        v.update(survx=300, jitter=1, seed=rng.randint(1, 10**6))
    return v


def v_dschain(rng):  # davinci dschain.py variant() (x_ds), beam 1000 egain 0.004 as in its defaults
    cp = rng.choice([('16', '2'), ('16', '2'), ('12', '1.5'), ('20', '2.5')])
    return {'beam': 1000, 'egain': 0.004, 'lam': rng.choice(['0,0.004,0.01,0.02', '0,0.004,0.01', '0,0.002,0.006,0.012']),
            'kickmin': rng.choice(['0', '10', '10', '11']), 'cellpos': cp[0], 'cellvel': cp[1],
            'jitter': rng.choice(['0', '0', '0.3']), 'seed': rng.randint(1, 10**6), 'shadow': rng.choice(['2', '2', '3']),
            'surv': rng.choice(['8', '12']), 'credw': rng.choice(['0', '0', '0.3']),
            'trackfrac': rng.choice(['0.15', '0.3', '0.3', '0.45']), 'shh': rng.choice(['30', '40', '40', '60']),
            'rollpre': '4', 'shmode': rng.choice(['1', '2']), 'kcred': rng.choice(['0', '0.5', '1']),
            'kready': rng.choice(['0', '3', '6']), 'sinkh': rng.choice(['0', '0', '40']), 'retro': rng.choice(['3', '4'])}


TSINKS = '1156,1285,1443,1793,2143,2465'  # Teero's speed minima (davinci lnsinc.py)


def run_job(rng, wid):
    with lock:
        text = open(best_file).read()
        cur = state['best']
        state['jobs'] += 1
        job = state['jobs']
    lines = text.splitlines()
    end_rt = cur
    cut = rng.randint(CUTMIN, end_rt - (3 if PRE else 40))
    src = f'{D}/w{wid}_best.txt'; pf = f'{D}/w{wid}_pre.txt'; out = f'{D}/w{wid}_out'
    open(src, 'w').write(text)
    open(pf, 'w').write('\n'.join(lines[:cut + 68]) + '\n')
    for f in os.listdir(D):
        if f.startswith(f'w{wid}_out'):
            os.remove(f'{D}/{f}')
    remain = T_END - time.time()
    if remain < 20:
        return
    if TOOL == 'ddsearch':
        v = v_ddsearch(rng)
        cmd = [f'{B["far"]}/ddsearch', MAP, f'best={src}', f'cut={cut}', 'threads=1', f'out={out}.txt']
        cand = f'{out}.txt'
    elif TOOL in ('seg-cannon', 'segf-cray'):
        v = v_lns3(rng)
        tr = own_track(text)
        common = [f'prefix={pf}', f'ref={tr}', 'gate=grenade', 'horizon=150', 'threads=1', 'quiet=1', 'survevery=10',
                  f'maxticks={2 * (end_rt - cut) + 60}', f'out={out}_']
        if TOOL == 'seg-cannon':
            cmd = [f'{B["cannon"]}/seg', MAP, f'anchor={src}'] + common
        else:
            cmd = [f'{B["cray"]}/segf', MAP, f'inc={src}'] + common
        cand = f'{out}_0.txt'
    elif TOOL in ('segf-dav', 'segfx-cray'):
        tr = own_track(text)
        v = v_lnsinc(rng) if TOOL == 'segf-dav' else v_lnsinc_survx(rng)
        bin_ = f'{B["dav"]}/segf' if TOOL == 'segf-dav' else f'{B["cray"]}/segf'
        cmd = [bin_, MAP, f'prefix={pf}', f'inc={src}', 'gate=finish', 'horizon=150', 'threads=1', 'quiet=1',
               f'maxticks={end_rt - cut + 80}', f'ref={tr}', f'out={out}_']
        cand = f'{out}_0.txt'
    elif TOOL == 'xds-dav':
        v = v_dschain(rng)
        cmd = [f'{B["dav"]}/x_ds', MAP, f'inc={src}', f'cut={cut}', 'incforce=1', 'gate=finish', 'threads=1', 'verbose=0',
               f'out={out}.txt']
        cand = f'{out}.txt'
    else:
        raise SystemExit('unknown tool ' + TOOL)
    cmd += [f'{k}={x}' for k, x in v.items()]
    t0 = time.time()
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, timeout=remain, cwd=W).stdout
    except subprocess.TimeoutExpired:
        log(f'w{wid} job {job} cut {cut}: timeout at budget end')
        return
    dt = time.time() - t0
    vs = ' '.join(f'{k}={x}' for k, x in v.items() if k not in ('sinks',))
    if not os.path.exists(cand):
        log(f'w{wid} job {job} cut {cut}: no result ({dt:.0f}s) [{vs}] {res.strip().splitlines()[-1] if res.strip() else ""}')
        return
    m = metric(cand)
    with lock:
        cur = state['best']
        if m is not None and m < cur:
            open(best_file, 'w').write(open(cand).read())
            open(f'{D}/best_{m}.txt', 'w').write(open(cand).read())
            state['best'] = m
            state['gains'].append((time.time() - (T_END - MIN * 60), m))
            tag = f'** NEW BEST {m} ** (server-checked)'
        else:
            tag = f'{m} (best {cur})'
    log(f'w{wid} job {job} cut {cut}: {tag} ({dt:.0f}s) [{vs}]')


def worker(wid):
    rng = random.Random(int(kw.get('seed', 1)) * 1000 + wid)
    while time.time() < T_END - 20:
        run_job(rng, wid)


ths = [threading.Thread(target=worker, args=(i,)) for i in range(NW)]
for t in ths:
    t.start()
for t in ths:
    t.join()
log(f'END {TOOL}: best {state["best"]} after {state["jobs"]} jobs; gains {state["gains"]}')
