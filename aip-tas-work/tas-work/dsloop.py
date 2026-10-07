#!/usr/bin/env python3
"""Outer loop: x_ds chains from rotating start points (the incumbent's speed minima), each to the finish with the current
best run as incumbent; a finish improvement is checked on the real server (TasReplay) and becomes the new best.
usage: dsloop.py BEST NAME [workers=3] [n=8] [beam=1000] [hours=4] [starts=auto|a,b,c]"""
import os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
best, name = sys.argv[1], sys.argv[2]
kw = dict(a.split('=', 1) for a in sys.argv[3:])
NW, N, BEAM = kw.get('workers', '3'), kw.get('n', '8'), kw.get('beam', '1000')
HOURS = float(kw.get('hours', 4))
LOG = f'runs/dsloop_{name}.log'
t_end = time.time() + HOURS * 3600


def log(s):
    print(s, flush=True)
    with open(LOG, 'a') as f:
        f.write(time.strftime('%H:%M:%S ') + s + '\n')


def finish_of(run):
    out = subprocess.run(['./srvfin.sh', run], capture_output=True, text=True).stdout
    m = re.search(r'finish tick (\d+) -> (\d+) ticks', out)
    ok = m and 'frozen ticks 0' in out and 'DOUBLE' not in out and 'died' not in out and '[       OK ]' in out
    return int(m.group(2)) if ok else None


src = open('dschain.py').read()
exec(src[src.index('def auto_sinks'):src.index("if kw.get('sinks') == 'auto'")])

F = finish_of(best)
log(f'loop {name}: best {best} server finish {F}')
it = 0
done_starts = set()
while time.time() < t_end:
    sinks = auto_sinks(best, 1060)
    starts = [1030] + sinks[:-2]
    if 'starts' in kw and kw['starts'] != 'auto':
        starts = [int(x) for x in kw['starts'].split(',')]
    # next start not tried on this best yet (latest first: cheaper, and late gains are kept by later chains)
    cand = [s for s in sorted(starts, reverse=True) if (best, s) not in done_starts]
    if not cand:
        log('all starts tried on this best; stop')
        break
    st = cand[0]
    done_starts.add((best, st))
    it += 1
    tag = f'{name}_{it:03d}'
    t0 = time.time()
    out = subprocess.run(['python3', 'dschain.py', best, str(st), tag, f'n={N}', f'workers={NW}', f'beam={BEAM}', 'egain=0.002',
                          'sinks=auto'], capture_output=True, text=True).stdout
    m = re.search(r'FINISH (\d+) \(incumbent (\d+)\) -> (\S+)', out)
    if not m:
        log(f'{tag} start {st}: no finish ({time.time() - t0:.0f}s)')
        continue
    fin, run = int(m.group(1)), m.group(3)
    log(f'{tag} start {st}: finish {fin} (best {F}) ({time.time() - t0:.0f}s)')
    if fin < F:
        sf = finish_of(run)
        if sf is not None and sf < F:
            F = sf
            best = f'kog_full_{F}.txt'
            subprocess.run(['cp', run, best])
            log(f'NEW BEST {F} (server-checked) -> {best}')
        else:
            log(f'  server check failed / not better: {sf}')
