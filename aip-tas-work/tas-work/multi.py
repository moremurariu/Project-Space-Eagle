#!/usr/bin/env python3
"""Run seg jobs in parallel. usage: multi.py JOBFILE OUTDIR [workers=4]
JOBFILE lines: TAG PREFIX GATE seg-options...   -> OUTDIR/TAG_0.txt, summary in OUTDIR/multi.log"""
import subprocess, sys, os, re, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
SEG = os.environ.get('SEGBIN', '../ddnet/build-sim/segf')
jobs = [l.split() for l in open(sys.argv[1]) if l.strip() and not l.startswith('#')]
D = sys.argv[2]; os.makedirs(D, exist_ok=True)
NW = int(sys.argv[3]) if len(sys.argv) > 3 else 4
LOG = f'{D}/multi.log'
def run(j):
    tag, prefix, gate, *opts = j
    t0 = time.time()
    cmd = [SEG, 'AiP-Gores.map', f'prefix={prefix}', 'ref=teero_track.txt', f'gate={gate}', 'horizon=150', 'threads=1',
           'quiet=1', 'survevery=10', f'out={D}/{tag}_'] + opts
    if not any(o.startswith('maxticks=') for o in opts):
        cmd.append('maxticks=400')
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    m = re.search(r'GATE rt (\d+) value ([-\d.]+) Ee (-?\d+)', res)
    s = res.strip().splitlines()[0] if res.strip() else ''
    line = f"{tag} {'GATE rt ' + m.group(1) + ' Ee ' + m.group(3) if m else 'NOGATE'} ({time.time()-t0:.0f}s) {' '.join(opts)}"
    with open(LOG, 'a') as f:
        f.write(time.strftime('%H:%M:%S ') + line + '\n')
    return line
with ThreadPoolExecutor(NW) as ex:
    for l in ex.map(run, jobs):
        print(l, flush=True)
