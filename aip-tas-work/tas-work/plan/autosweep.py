#!/usr/bin/env python3
"""autosweep.py DIR [cores=4] [sections=a:b,c:d,...] [vsets=f1,f2] [xs="allcuts=1 ..."] [cycle0=N]: endless xsweep cycles on tas-work/kog_full_best.txt.
Each cycle runs one xsweep round per variant set (dschain_variants_rf, _rf_hr, _rf2) with a new seed offset over all
sections; a better server-checked run is copied to tas-work/kog_full_<ticks>.txt and kog_full_best.txt and logged
('NEW <ticks>' in DIR/auto.log). Stop it with kill."""
import glob, os, re, shutil, subprocess, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
d = os.path.abspath(sys.argv[1])
kw = dict(a.split('=', 1) for a in sys.argv[2:])
CORES = kw.get('cores', '4')
SECS = kw.get('sections', '1007:1095,1095:1270,1270:1445,1420:1560,1590:1765,1700:1875,1825:2000,2000:2175,'
              '2175:2350,2350:2525,2430:2570').split(',')
VSETS = kw.get('vsets', 'dschain_variants_rf.txt,dschain_variants_rf_hr.txt,dschain_variants_rf2.txt').split(',')
XS = kw.get('xs', '').split()  # extra xsweep args, e.g. xs="allcuts=1 cutsp=4"

os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'auto.log'), 'a')


def log(m):
    m = time.strftime('%H:%M:%S ') + m
    print(m, flush=True)
    LOG.write(m + '\n')
    LOG.flush()


def server_finish(path):
    r = subprocess.run(['bash', os.path.join(TW, 'srvfin.sh'), path], capture_output=True, text=True).stdout
    m = re.search(r'-> (\d+) ticks', r)
    f = re.search(r'frozen ticks (\d+)', r)
    return int(m.group(1)) if m and f and f.group(1) == '0' and 'DOUBLE' not in r else None


best = server_finish(os.path.join(TW, 'kog_full_best.txt'))
log(f'autosweep: start at {best}, sections {SECS}')
cycle = int(kw.get('cycle0', 0))
while True:
    for vi, vs in enumerate(VSETS):
        sd = os.path.join(d, f'c{cycle}_{vi}')
        seed0 = 1000 * (cycle + 1) + 10 * vi
        cur = os.path.join(TW, 'kog_full_best.txt')
        log(f'cycle {cycle} {vs} seed0 {seed0} on {best}')
        subprocess.run([sys.executable, os.path.join(HERE, 'xsweep.py'), cur, sd] + SECS +
                       [f'cores={CORES}', 'rounds=1', f'seed0={seed0}', f'variants={os.path.join(HERE, vs)}'] + XS,
                       capture_output=True, text=True)
        cands = sorted((int(re.search(r'best_(\d+)', f).group(1)), f) for f in glob.glob(os.path.join(sd, 'best_*.txt')))
        if cands and (best is None or cands[0][0] < best):
            n, f = cands[0]
            srv = server_finish(f)
            if srv is not None and (best is None or srv < best):
                best = srv
                shutil.copy(f, os.path.join(TW, f'kog_full_{srv}.txt'))
                shutil.copy(f, os.path.join(TW, 'kog_full_best.txt'))
                log(f'NEW {srv} <- {f}')
    cycle += 1
