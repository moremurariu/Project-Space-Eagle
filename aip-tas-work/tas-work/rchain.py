#!/usr/bin/env python3
"""rchain.py START_PREFIX NAME [window=150] [commit=75] [beam=6000] [lam=0.003] [steps=N] [variants=a,b,..]: window chain,
best of a few variants per window (free / retro shots / retro + loadres), sequential, threads=4. State in runs/rc/NAME/."""
import os, re, subprocess, sys, time
HERE = os.path.dirname(os.path.abspath(__file__)); os.chdir(HERE)
MAP = 'AiP-Gores.map'
args = dict(a.split('=', 1) for a in sys.argv[3:])
SEG = '../ddnet/build-sim/' + args.get('bin', 'segf')
D = f'runs/rc/{sys.argv[2]}'; os.makedirs(D, exist_ok=True)
WIN = int(args.get('window', 150)); COM = int(args.get('commit', 75)); BEAM = int(args.get('beam', 6000)); LAM = float(args.get('lam', 0.003))
STEPS = int(args.get('steps', 99))
FINISH_K = 2539
BASE = (f'horizon=150 beam={BEAM} threads=4 quiet=1 survevery=2 survive=30 rothook=1 quant=1 ghost=3 hnow=1000 ghoste=0.02 '
        'sinks=1156,1285,1443,1793,2143,2465 kcredit=2 kready=10 ghostsink=1500 latpen=0.2 latdz=24').split()
VAR = {'free1': 'jitter=1 seed=1', 'free2': 'jitter=1 seed=2', 'rt1': 'retro=1 jitter=1 seed=1', 'rt1b': 'retro=1 jitter=1 seed=2',
       'rlr2': 'retro=3 loadres=0.2 jitter=1 seed=1', 'rlr4': 'retro=3 loadres=0.4 jitter=1 seed=2', 'rlr4b': 'retro=3 loadres=0.4 jitter=1 seed=3',
       'rlr2q': 'retro=3 loadres=0.2 quota=30 jitter=1 seed=1', 'rlr4q': 'retro=3 loadres=0.4 quota=30 jitter=1 seed=2',
       'free3': 'jitter=1 seed=3', 'rt1c': 'retro=1 jitter=1 seed=3', 'rlr2qb': 'retro=3 loadres=0.2 quota=30 jitter=1 seed=3', 'rt3q': 'retro=3 quota=30 jitter=1 seed=4',
       'sv0': 'survevery=0 jitter=1 seed=1', 'sv0b': 'survevery=0 jitter=1 seed=2', 'rt1sv0': 'retro=1 survevery=0 jitter=1 seed=3',
       'sx1': 'survx=300 jitter=1 seed=1', 'sx2': 'survx=300 jitter=1 seed=2', 'rt1sx': 'retro=1 survx=300 jitter=1 seed=3',
       'rlr2qsx': 'retro=3 loadres=0.2 quota=30 survx=300 jitter=1 seed=1'}
VLIST = args.get('variants', 'free1,rt1,rlr2,rlr4').split(',')
LOG = f'{D}/rc.log'
def log(m):
    open(LOG, 'a').write(time.strftime('%H:%M:%S ') + m + '\n'); print(m, flush=True)
def start(prefix):
    o = subprocess.run([SEG, MAP, f'prefix={prefix}', 'gate=finish', 'maxticks=0', 'quiet=1'], capture_output=True, text=True).stdout
    m = re.search(r'start: rt (-?\d+) .* \(teero tick (-?\d+)\)', o)
    return int(m.group(1)), int(m.group(2))
def seg(prefix, gate, commitk, tag, extra, maxticks):
    out = f'{D}/{tag}_'
    cmd = [SEG, MAP, f'prefix={prefix}', f'gate={gate}', f'maxticks={maxticks}', f'out={out}'] + BASE + extra.split()
    if commitk is not None: cmd.append(f'commitk={commitk}')
    o = subprocess.run(cmd, capture_output=True, text=True).stdout
    m = re.search(r'GATE rt (\d+) value [-\d.]+ Ee (-?\d+)', o)
    if not m: return None
    c = re.search(r'COMMIT \d+ inputs rt (\d+) ref \d+ \(teero (-?\d+)\) -> (\S+)', o)
    return {'rt': int(m.group(1)), 'ee': int(m.group(2)), 'full': out + '0.txt', 'commit': c.group(3) if c else None, 'tag': tag}
done = sorted((int(f[1:-4]), f) for f in os.listdir(D) if re.fullmatch(r'c\d+\.txt', f))
prefix = f'{D}/{done[-1][1]}' if done else sys.argv[1]; step = done[-1][0] if done else 0
while step < STEPS:
    rt0, k0 = start(prefix)
    final = k0 + WIN >= FINISH_K - 5
    gate = 'finish' if final else str(k0 + WIN); commitk = None if final else k0 + COM
    maxt = ((FINISH_K - k0) if final else WIN) * 2 + 60
    step += 1
    res = [r for r in (seg(prefix, gate, commitk, f's{step}{v}', VAR[v], maxt) for v in VLIST) if r]
    if not res:
        log(f'step {step}: from rt {rt0} k {k0}: all failed'); break
    best = min(res, key=lambda r: (r['rt'] - (0 if final else LAM * r['ee']), r['rt']))
    summ = ', '.join(f"{r['tag']}:{r['rt']}/{r['ee']}" for r in res)
    if final:
        open(f'{D}/final.txt', 'w').write(open(best['full']).read())
        log(f'step {step}: FINISH rt {best["rt"]} [{best["tag"]}] ({summ})'); break
    log(f'step {step}: from rt {rt0} k {k0} gate {gate}: best rt {best["rt"]} (lead {int(gate) - 3 - best["rt"]:+d}) [{best["tag"]}] ({summ})')
    dst = f'{D}/c{step}.txt'; open(dst, 'w').write(open(best['commit']).read()); prefix = dst
