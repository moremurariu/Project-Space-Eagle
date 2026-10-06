#!/usr/bin/env python3
"""Receding-horizon chain with x_ds: windows of W incumbent ticks (progress along the incumbent's path), commit the
first C, best of N randomized variants per window (parallel). The first window starts from the incumbent itself
(forced lineage), later ones from the committed prefix with the incumbent as reference + shadow inputs.
usage: dschain.py INC CUT NAME [W=200] [C=100] [n=8] [workers=4] [beam=1000] [egain=0.004] [extra x_ds args]"""
import os, random, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
BIN = os.environ.get('DSBIN', '../ddnet/build-sim/x_ds')
inc, cut, name = sys.argv[1], int(sys.argv[2]), sys.argv[3]
kw, extra = {}, []
for a in sys.argv[4:]:
    k, _, v = a.partition('=')
    if k in ('W', 'C', 'n', 'workers', 'beam', 'egain', 'seed0', 'finishat', 'sinks'):
        kw[k] = v
    else:
        extra.append(a)
W, C, N, NW = int(kw.get('W', 200)), int(kw.get('C', 100)), int(kw.get('n', 8)), int(kw.get('workers', 4))
BEAM, EG, S0 = kw.get('beam', '1000'), float(kw.get('egain', 0.004)), int(kw.get('seed0', 1))
D = f'runs/dsc/{name}'
os.makedirs(D, exist_ok=True)
LOG = f'{D}/chain.log'


def log(s):
    print(s, flush=True)
    with open(LOG, 'a') as f:
        f.write(time.strftime('%H:%M:%S ') + s + '\n')


# incumbent finish (race tick) for the last window
res = subprocess.run([BIN, 'AiP-Gores.map', f'inc={inc}', f'cut={cut}', 'gate=finish', 'maxsteps=0', 'verbose=0'],
                     capture_output=True, text=True).stdout
FIN = int(re.search(r'incumbent finish rt (\d+)', res).group(1))
FINAT = int(kw.get('finishat', FIN - 60))
log(f'chain {name}: inc {inc} (finish {FIN}) cut {cut} W {W} C {C} n {N} beam {BEAM} egain {EG} extra {" ".join(extra)}')


def variant(i, w):
    r = random.Random(w * 1009 + i * 7919 + S0)
    cp = r.choice([('16', '2'), ('16', '2'), ('12', '1.5'), ('20', '2.5')])
    return {
        'lam': r.choice(['0,0.004,0.01,0.02', '0,0.004,0.01', '0,0.002,0.006,0.012']),
        'kickmin': r.choice(['0', '10', '10', '11']),
        'cellpos': cp[0], 'cellvel': cp[1],
        'jitter': r.choice(['0', '0', '0.3']),
        'seed': str(i + S0 + 100 * w),
        'shadow': r.choice(['2', '2', '3']),
        'surv': r.choice(['8', '12']),
        'credw': r.choice(['0', '0', '0.3']),
        'trackfrac': r.choice(['0.15', '0.3', '0.3', '0.45']),
        'shh': r.choice(['30', '40', '40', '60']),
        'rollpre': '4',
        'shmode': r.choice(['1', '2']),
    }


def auto_sinks(run, after):
    out = subprocess.run(['../ddnet/build-sim/x_trace', 'AiP-Gores.map', run], capture_output=True, text=True).stdout
    V = [(int(a[2]), float(a[7])) for a in (l.split() for l in out.splitlines()) if a and a[0] == 'T' and int(a[2]) >= after]
    sp = [v for _, v in V]
    rt = [r for r, _ in V]
    res = []
    for i in range(15, len(sp) - 15):
        if sp[i] == min(sp[i - 15:i + 16]):
            l = max(sp[max(0, i - 60):i])
            r = max(sp[i + 1:i + 61])
            if min(l, r) - sp[i] > 8 and sp[i] < 32 and (not res or rt[i] - res[-1] > 40):
                res.append(rt[i])
    return res


if kw.get('sinks') == 'auto':
    SINKS = auto_sinks(inc, cut + 30)
    log(f'auto sinks: {SINKS}')
else:
    SINKS = [int(x) for x in kw['sinks'].split(',')] if 'sinks' in kw else None
prefix = None
K = cut  # committed progress
w = 0
while True:
    if SINKS:
        nxt = [x for x in SINKS if x > K + 20]
        commit = nxt[0] if nxt else FIN
        last = len(nxt) < 2
        gate = 'finish' if last else f'rt{nxt[1]}'
        W = (nxt[1] - K) if not last else 0
    else:
        last = K + W >= FINAT
        gate = 'finish' if last else f'rt{K + W}'
        commit = K + C

    def run(i):
        v = variant(i, w)
        tag = f'w{w:02d}_v{i:02d}'
        out = f'{D}/{tag}.txt'
        args = [BIN, 'AiP-Gores.map', f'inc={inc}', f'gate={gate}', 'threads=1', 'verbose=0', f'beam={BEAM}',
                f'egain={EG}', f'out={out}'] + [f'{k}={x}' for k, x in v.items()] + extra
        if prefix is None:
            args += [f'cut={cut}', 'incforce=1']
        else:
            args += [f'prefix={prefix}', f'anc={anc}']
        if not last:
            args.append(f'commitk={commit}')
        t0 = time.time()
        r = subprocess.run(args, capture_output=True, text=True).stdout
        m = re.search(r'GATE t ([\d.]+) \(incumbent ([\d.]+)\) E (-?\d+)', r)
        mv = re.search(r'verify \(CTasGame\): gate rt (-?\d+) end rt (-?\d+) geo \S+ finish (-?\d+) dead (\d)', r)
        if not m or not mv or mv.group(4) != '0':
            return (1e9, tag, out, 'NOGATE', time.time() - t0)
        t = float(m.group(1))
        if last:
            fin = int(mv.group(3))
            if fin < 0:
                return (1e9, tag, out, 'NOFINISH', time.time() - t0)
            return (float(fin), tag, out, f'finish {fin}', time.time() - t0)
        if not os.path.exists(out + '.c'):
            return (1e9, tag, out, 'NOCOMMIT', time.time() - t0)
        me = re.search(r'incumbent reaches it at rt [\d.]+ with E (-?\d+)', r)
        e, eref = float(m.group(3)), float(me.group(1)) if me else 0.0
        val = t - EG * (e - eref)
        return (val, tag, out, f't {t:.2f} E {e:.0f} (ref {eref:.0f}) value {val:.2f}', time.time() - t0)

    with ThreadPoolExecutor(NW) as ex:
        R = list(ex.map(run, range(N)))
    R.sort()
    for v, tag, out, desc, dt in R:
        log(f'  w{w:02d} {tag} {desc} ({dt:.0f}s)')
    best = R[0]
    if best[0] >= 1e9:
        log(f'window {w}: no result, stop')
        break
    if last:
        log(f'FINISH {best[0]:.0f} (incumbent {FIN}) -> {best[2]}')
        break
    lead = (K + W) - float(re.search(r't ([\d.]+)', best[3]).group(1))
    log(f'window {w}: gate {gate} best {best[1]} {best[3]} lead {lead:+.2f}, commit at progress {commit}')
    prefix = best[2] + '.c'
    anc = best[2]
    K = commit
    w += 1
