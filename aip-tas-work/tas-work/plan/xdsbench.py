#!/usr/bin/env python3
"""xdsbench.py DIR RUN CUTS [len=150] [par=4] [beam=4000] [vars=plan/dschain_variants_rf.txt] [only=X,..] [X=extra args] [Y=...]
Search-quality benchmark: x_ds from RUN's own state at each cut to the gate cut+len (incumbent = RUN), every variant,
once per setting (X / Y / ... = extra x_ds args; 'base' = none). A search that cannot even follow its incumbent ends
behind it; the table is the gate time minus the incumbent's (negative = ahead). Writes DIR/bench.txt."""
import os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
TW = os.path.dirname(HERE)
BIN = os.path.join(TW, '..', 'ddnet', 'build-sim')
MAP = os.path.join(TW, 'AiP-Gores.map')
d, run = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
cuts = [int(c) for c in sys.argv[3].split(',')]
kw = {}
sets = {'base': ''}
for a in sys.argv[4:]:
    k, v = a.split('=', 1)
    if k in ('len', 'par', 'beam', 'vars', 'only'):
        kw[k] = v
    else:
        sets[k] = v
L, PAR, BEAM = int(kw.get('len', 150)), int(kw.get('par', 4)), kw.get('beam', '4000')
VAR = [l.strip() for l in open(os.path.join(TW, kw.get('vars', 'plan/dschain_variants_rf.txt'))) if l.strip() and not l.startswith('#')]
os.makedirs(d, exist_ok=True)
out = subprocess.run([os.path.join(BIN, 'x_trace'), MAP, run, '2400'], capture_output=True, text=True).stdout
FIN = min(int(l.split()[2]) for l in out.splitlines() if l.startswith('T ') and 'FINISH' in l)
lines = open(run).readlines()


def one(job):
    c, s, i = job
    pre = os.path.join(d, 'c%d.txt' % c)
    g = c + L
    gate = 'gate=finish' if g >= FIN - 3 else 'gate=rt%d' % g
    o = os.path.join(d, 'r_%d_%s_v%d.txt' % (c, s, i))
    if not os.path.exists(o + '.log') or 'GATE' not in open(o + '.log').read():
        cmd = [os.path.join(BIN, 'x_ds'), MAP, 'inc=' + run, 'prefix=' + pre, gate, 'incforce=0', 'threads=1', 'beam=' + BEAM,
               'verbose=0', 'out=' + o] + VAR[i].split() + sets[s].split()
        with open(o + '.log', 'w') as lf:
            subprocess.run(cmd, stdout=lf, stderr=subprocess.STDOUT)
    m = re.search(r'GATE t ([0-9.]+)', open(o + '.log').read())
    return job, (float(m.group(1)) - min(g, FIN)) if m else None


for c in cuts:
    with open(os.path.join(d, 'c%d.txt' % c), 'w') as f:
        f.writelines(lines[:c + 68])
jobs = [(c, s, i) for c in cuts for s in sets for i in range(len(VAR))]
if 'only' in kw:  # run only these settings (another bench in the same DIR runs the rest); the table needs all
    run_jobs = [j for j in jobs if j[1] in kw['only'].split(',')]
    with ThreadPoolExecutor(PAR) as ex:
        list(ex.map(one, run_jobs))
    sys.exit(0)
with ThreadPoolExecutor(PAR) as ex:
    res = dict(ex.map(one, jobs))
with open(os.path.join(d, 'bench.txt'), 'w') as f:
    f.write('settings: %s\n' % sets)
    f.write('cut   ' + ' '.join('%-28s' % s for s in sets) + '\n')
    tot = {s: [] for s in sets}
    for c in cuts:
        row = []
        for s in sets:
            v = [res[(c, s, i)] for i in range(len(VAR))]
            ok = [x for x in v if x is not None]
            tot[s].append(min(ok) if ok else 99)
            row.append('%-28s' % ('best %+6.2f [%s]' % (min(ok), ' '.join('%+.1f' % x if x is not None else 'X' for x in v)) if ok else 'NOGATE'))
        f.write('%-5d ' % c + ' '.join(row) + '\n')
    f.write('sum of best: ' + ' '.join('%s %+.2f' % (s, sum(tot[s])) for s in sets) + '\n')
print(open(os.path.join(d, 'bench.txt')).read())
