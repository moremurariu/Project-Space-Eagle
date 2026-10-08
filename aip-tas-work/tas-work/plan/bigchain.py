#!/usr/bin/env python3
"""bigchain.py INC CUT DIR [W=150] [C=75] [beam=40000] [cellpos=5] [cellvel=0.6] [threads=4] [variant=FILE|default]
Receding-horizon chain of high-resolution x_ds windows (fine cells, big beam) along INC's path. Window w runs from
the committed prefix to INC's progress at start+W (forced lineage = the previous window's line, so a window never
ends behind it) and commits the part up to INC's progress start+C. The last window runs to the finish.
Why: the polished runs came from beam 1000-5000 searches with 12-20 px / 1.5-2.5 px/t cells; at 5 px / 0.6 px/t and
beam 40000 the same x_ds variants find faster lines (U-turn 1: +2.2 ticks over 190 ticks, see NOTES)."""
import os, re, subprocess, sys, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from shots import shots
HERE = os.path.dirname(os.path.abspath(__file__)); TW = os.path.dirname(HERE); os.chdir(TW)
XDS = os.environ.get('XDSBIN', '../ddnet/build-sim/x_ds')
inc, cut, d = os.path.abspath(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
kw = dict(a.split('=', 1) for a in sys.argv[4:])
W, C = int(kw.get('W', 150)), int(kw.get('C', 75))
VAR = ('seed=21 lam=0,0.004,0.01,0.02 kickmin=10 shadow=2 surv=12 trackfrac=0.3 shh=40 rollpre=4 shmode=1 kcred=0.5 '
       'kready=3 retro=4 egain=0.004 rotfar=1 brakew=0.3')
if kw.get('variant', 'default') != 'default':
    VAR = open(kw['variant']).read().strip()
BIG = f"beam={kw.get('beam', 40000)} cellpos={kw.get('cellpos', 5)} cellvel={kw.get('cellvel', 0.6)} threads={kw.get('threads', 4)}"
os.makedirs(d, exist_ok=True)
LOG = open(os.path.join(d, 'chain.log'), 'a')
def log(m):
    m = time.strftime('%H:%M:%S ') + m; print(m, flush=True); LOG.write(m + '\n'); LOG.flush()
FIN = len(open(inc).read().splitlines()) - 68
RES = int(kw.get('reserve', 1))
PRE = [(f, e, fl) for f, e, fl in shots(inc) if fl >= 8]  # the incumbent's pre-fires (lobs)
prefix = os.path.join(d, 'p0.txt')
open(prefix, 'w').write('\n'.join(open(inc).read().splitlines()[:cut + 68]) + '\n')
anc = inc
start, w, lead = cut, 0, 0
log(f'bigchain inc={inc} (finish {FIN}) cut={cut} W={W} C={C} {BIG} var: {VAR}')
while True:
    last = start + W >= FIN - 10
    gate = 'finish' if last else f'rt{start + W}'
    out = os.path.join(d, f'w{w}.txt')
    args = [XDS, 'AiP-Gores.map', f'inc={inc}', f'prefix={prefix}', f'anc={anc}', f'gate={gate}', 'incforce=1', 'verbose=0',
            f'out={out}'] + VAR.split() + BIG.split()
    if not last:
        args.append(f'commitk={start + C}')
    # keep the reload free for the incumbent's pre-fires in this window, shifted by our lead
    ns = []
    for f, e, fl in PRE:
        if RES and start - 5 <= f <= start + W:
            a, b = f - lead - 26, f - lead - 3
            if b > start - lead:
                ns += [max(a, start - lead + 1), b]
    if ns:
        args.append('noshot=' + ','.join(str(x) for x in ns))
    t0 = time.time()
    r = subprocess.run(args, capture_output=True, text=True).stdout
    m = re.search(r'GATE t ([\d.]+) \(incumbent ([\d.]+)\) E (-?\d+)', r)
    v = re.search(r'verify \(CTasGame\): gate rt (-?\d+) end rt (-?\d+) geo \S+ finish (-?\d+) dead (\d)', r)
    if not m or not v or v.group(4) != '0':
        log(f'window {w} ({start}->{gate}) failed:\n' + r[-1500:]); sys.exit(1)
    cm = re.search(r'commit: \d+ inputs \(progress (\d+) reached at rt (-?\d+)\)', r)
    log(f'window {w} {start}->{gate} lead {lead} noshot {ns}: GATE {m.group(1)} (incumbent {m.group(2)}) E {m.group(3)} finish {v.group(3)} [{time.time()-t0:.0f}s]')
    if last:
        log(f'FINISH {v.group(3)} -> {out}'); break
    prefix, anc = out + '.c', out
    if cm: lead = int(cm.group(1)) - int(cm.group(2))
    start += C; w += 1
