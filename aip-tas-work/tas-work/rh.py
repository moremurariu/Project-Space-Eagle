#!/usr/bin/env python3
"""Receding-horizon chain over the whole route with `seg`.

From the current prefix, search to a gate WINDOW Teero ticks ahead with several model variants in parallel,
keep the variant that reaches the gate first (then most energy), commit only its first COMMIT Teero ticks,
repeat until the finish. On a dead end (no variant reaches the gate), step back one commit and retry wider.

usage: rh.py START_PREFIX NAME [window=250] [commit=100] [beam=20000]   (state in runs/rh/NAME/)
"""
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'
MAP = 'AiP-Gores.map'
FINISH_K = 2536
SINKS = '315,550,725,900'

args = dict(a.split('=', 1) for a in sys.argv[3:])
WINDOW = int(args.get('window', 250))
COMMIT = int(args.get('commit', 100))
BEAM = int(args.get('beam', 20000))
D = f'runs/rh/{sys.argv[2]}'
os.makedirs(D, exist_ok=True)
LOG = f'{D}/rh.log'

VARIANTS = [
    'ghost=1 hnow=600 ghoste=0.02',
    'ghost=1 hnow=600 ghoste=0.02 pjc=250 pgc=250',
    'ghost=1 hnow=300 ghoste=0.04',
    f'hnow=300 sinks={SINKS} pjc=200',
]
NVAR = int(args.get('nvar', 4))
SEL_LAMBDA = float(args.get('sel', 0.02))  # gate choice: race ticks minus this many ticks per unit of energy
# after the grenade pickup: shot variants (near point-blank shots, longer shots, more aims, more energy credit)
POST = [
    'ghost=1 hnow=600 ghoste=0.02',
    'ghost=1 hnow=600 ghoste=0.04',
    'ghost=1 hnow=400 ghoste=0.02 fireangles=48',
    'ghost=1 hnow=600 ghoste=0.02 firerange=220 firelook=16',
]
POST_SINKS = '1156,1443,1793,2143,2465'
if args.get('vset') == 'g2':
    VARIANTS = [
        'ghost=2 hnow=600 ghoste=0.02',
        'ghost=2 hnow=300 ghoste=0.02',
        'ghost=2 hnow=600 ghoste=0.02 pjc=250 pgc=250',
        'ghost=2 hnow=400 ghoste=0.03',
    ]
    POST = [
        f'ghost=2 hnow=600 ghoste=0.02 sinks={POST_SINKS} ghostsink=1500',
        'ghost=2 hnow=300 ghoste=0.02',
        f'ghost=2 hnow=400 ghoste=0.03 sinks={POST_SINKS} ghostsink=1000 fireangles=48',
        'ghost=2 hnow=600 ghoste=0.02 firerange=220 firelook=16',
    ]
if args.get('vset') == 'g3':
    VARIANTS = [
        'ghost=1 hnow=600 ghoste=0.02',
        'ghost=1 hnow=600 ghoste=0.02 pjc=250 pgc=250',
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={SINKS} ghostsink=1500',
        'ghost=1 hnow=300 ghoste=0.04',
    ]
    POST = [
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={POST_SINKS} ghostsink=1500 kcredit=2 kready=10',
        'ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10',
        f'ghost=3 hnow=1000 ghoste=0.01 sinks={POST_SINKS} ghostsink=1500',
        f'ghost=3 hnow=800 ghoste=0.02 sinks={POST_SINKS} ghostsink=1000 kcredit=1 kready=4 fireangles=48',
    ]
    WIDE = [
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={POST_SINKS} ghostsink=1500 kcredit=2 kready=10 beam=40000',
        f'ghost=3 hnow=600 ghoste=0.02 sinks={SINKS},{POST_SINKS} ghostsink=1500 angles=96',
        'ghost=1 hnow=600 ghoste=0.01 beam=40000 angles=96',
        f'ghost=3 hnow=1200 ghoste=0.03 sinks={SINKS},{POST_SINKS} ghostsink=2000 beam=40000',
    ]
if args.get('vset') == 'g4':
    PS2 = '1156,1285,1443,1793,2143,2465'
    SV = 'beam=10000 survevery=2 survive=30'
    PF = 'prefire=1 padaims=48 padtop=300 padrange=30 latpen=0.02 latdz=48'
    POST = [
        f'{SV} ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 {PF}',
        f'{SV} ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 kcredit=2 kready=10',
        f'{SV} ghost=1 hnow=600 ghoste=0.02 {PF}',
        f'{SV} ghost=3 hnow=1000 ghoste=0.01 sinks={PS2} ghostsink=1500 angles=128 hookdedup=0',
    ]
    WIDE = [
        f'beam=20000 survevery=1 survive=40 ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500',
        f'beam=20000 survevery=1 survive=40 ghost=3 hnow=1500 ghoste=0.0 sinks={PS2} brake=2',
        f'beam=10000 survevery=1 survive=50 ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10',
        f'beam=20000 survevery=2 survive=40 ghost=3 hnow=1000 ghoste=0.01 sinks={PS2} ghostsink=1500 angles=128 hookdedup=0',
    ]
if args.get('vset') == 'r':
    # rotation-pulse hooks + whole-pixel speed terms (k401->651 benchmark from Teero's state: 653 vs 657, Teero 651)
    RQ = 'rothook=1 quant=1'
    VARIANTS = [
        f'ghost=1 hnow=600 ghoste=0.02 angles=128 hookdedup=0 {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {RQ}',
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={SINKS} ghostsink=1500 angles=128 hookdedup=0 {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 angles=128 hookdedup=0 pjc=250 pgc=250 {RQ}',
    ]
if args.get('vset') == 'r2':
    # 8 variants: what works is section dependent (k401 benchmark: base 653, tan+cpos8 657; U-turn k690-790: tan+cpos8
    # keeps 170 more energy)
    RQ = 'rothook=1 quant=1'
    A = 'angles=128 hookdedup=0'
    VARIANTS = [
        f'ghost=1 hnow=600 ghoste=0.02 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} dirmode=tan cpos=8 cvel=1 {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {RQ}',
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={SINKS} ghostsink=1500 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} pjc=250 pgc=250 {RQ}',
        f'ghost=1 hnow=300 ghoste=0.01 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} hookla=10 {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 cpos=8 cvel=1 dirmode=tan pjc=250 pgc=250 {RQ}',
    ]
    NVAR = int(args.get('nvar', 8))
if args.get('vset') == 'r3':
    RQ = 'rothook=1 quant=1'
    A = 'angles=128 hookdedup=0'
    LP = 'latpen=0.05 latdz=24'
    VARIANTS = [
        f'ghost=1 hnow=600 ghoste=0.02 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} dirmode=tan cpos=8 cvel=1 {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} cpos=8 cvel=1 {LP} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} {LP} {RQ}',
        f'ghost=3 hnow=1000 ghoste=0.02 sinks={SINKS} ghostsink=1500 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} pjc=250 pgc=250 {RQ}',
        f'ghost=1 hnow=300 ghoste=0.01 {A} {RQ}',
        f'ghost=1 hnow=600 ghoste=0.02 {A} pjc=250 pgc=250 cpos=8 cvel=1 {LP} {RQ}',
    ]
    NVAR = int(args.get('nvar', 8))
if args.get('vset') == 'p5':
    # post-grenade: g4's settings (ghost=3 hairpin survival, prefire / kick credit) + rotation pulses and whole pixels,
    # 8 variants (k-chains on KoG); sel after the grenade via selpost
    PS2 = '1156,1285,1443,1793,2143,2465'
    SV = 'beam=10000 survevery=2 survive=30'
    PF = 'prefire=1 padaims=48 padtop=300 padrange=30 latpen=0.02 latdz=48'
    RQ = 'rothook=1 quant=1'
    POST = [
        f'{SV} ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 {PF} {RQ}',
        f'{SV} ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 kcredit=2 kready=10 {RQ}',
        f'{SV} ghost=1 hnow=600 ghoste=0.02 {PF} {RQ}',
        f'{SV} ghost=3 hnow=1000 ghoste=0.01 sinks={PS2} ghostsink=1500 angles=128 hookdedup=0 {RQ}',
        f'{SV} ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10 angles=128 hookdedup=0 {RQ}',
        f'{SV} ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 {PF}',
        f'{SV} ghost=1 hnow=300 ghoste=0.01 kcredit=1 kready=4 {RQ}',
        f'{SV} ghost=3 hnow=600 ghoste=0.0 sinks={PS2} brake=2 kcredit=2 kready=10 {RQ}',
    ]
    WIDE = [
        f'beam=20000 survevery=1 survive=40 ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500 {RQ}',
        f'beam=20000 survevery=1 survive=40 ghost=3 hnow=1500 ghoste=0.0 sinks={PS2} brake=2 {RQ}',
        f'beam=10000 survevery=1 survive=50 ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10 {RQ}',
        f'beam=20000 survevery=2 survive=40 ghost=3 hnow=1000 ghoste=0.01 sinks={PS2} ghostsink=1500 angles=128 hookdedup=0',
    ]
    NVAR = int(args.get('nvar', 8))
if args.get('vset') == 'p6':
    # post-grenade with strong line following (latpen): from states weaker than Teero's the beam strays off his line
    PS2 = '1156,1285,1443,1793,2143,2465'
    SV = 'beam=10000 survevery=2 survive=30'
    RQ = 'rothook=1 quant=1'
    G3 = f'ghost=3 hnow=1000 ghoste=0.02 sinks={PS2} ghostsink=1500'
    POST = [
        f'{SV} {G3} kcredit=2 kready=10 {RQ} latpen=0.1 latdz=32',
        f'{SV} {G3} kcredit=2 kready=10 {RQ} latpen=0.2 latdz=24',
        f'{SV} ghost=3 hnow=600 ghoste=0.0 sinks={PS2} brake=2 kcredit=2 kready=10 {RQ} latpen=0.1 latdz=32',
        f'{SV} {G3} prefire=1 padaims=48 padtop=300 padrange=30 {RQ} latpen=0.1 latdz=32',
        f'{SV} {G3} kcredit=2 kready=10 {RQ} latpen=0.05 latdz=32',
        f'{SV} ghost=3 hnow=1000 ghoste=0.01 sinks={PS2} ghostsink=1500 angles=128 hookdedup=0 {RQ} latpen=0.1 latdz=32',
    ]
    WIDE = [
        f'beam=20000 survevery=1 survive=40 {G3} {RQ} latpen=0.1 latdz=32',
        f'beam=20000 survevery=1 survive=40 ghost=3 hnow=1500 ghoste=0.0 sinks={PS2} brake=2 {RQ} latpen=0.2 latdz=24',
        f'beam=10000 survevery=1 survive=50 ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10 {RQ} latpen=0.1 latdz=32',
        f'beam=20000 survevery=2 survive=40 {G3} {RQ} latpen=0.05 latdz=48',
    ]
    NVAR = int(args.get('nvar', 4))
STOP_K = int(args.get('stopk', 99999))
GREN_K = 990
WIDE_R = [
    'ghost=1 hnow=600 ghoste=0.02 beam=40000 angles=128 hookdedup=0 rothook=1 quant=1',
    'ghost=1 hnow=450 ghoste=0.03 angles=96 rothook=1 quant=1',
    f'ghost=3 hnow=1000 ghoste=0.02 sinks={SINKS} ghostsink=1500 beam=40000 rothook=1 quant=1',
    'ghost=1 hnow=600 ghoste=0.01 beam=40000 angles=96 rothook=1 quant=1',
]
WIDE = [
    'ghost=1 hnow=600 ghoste=0.02 beam=40000',
    'ghost=1 hnow=450 ghoste=0.03 angles=96',
    'ghost=1 hnow=600 ghoste=0.01 beam=40000 angles=96',
    f'hnow=300 sinks={SINKS} beam=40000',
]


def log(msg):
    with open(LOG, 'a') as f:
        f.write(time.strftime('%H:%M:%S ') + msg + '\n')
    print(msg, flush=True)


def start_k(prefix):
    out = subprocess.run([SEG, MAP, f'prefix={prefix}', 'gate=finish', 'maxticks=0', 'quiet=1'], capture_output=True, text=True).stdout
    m = re.search(r'start: rt (-?\d+) .* ref idx (\d+) \(teero tick (-?\d+)\)', out)
    return int(m.group(1)), int(m.group(3))


def run_variant(prefix, gate, commitk, variant, tag, maxticks):
    out = f'{D}/{tag}_'
    cmd = [SEG, MAP, f'prefix={prefix}', f'gate={gate}', 'horizon=150', f'beam={BEAM}', 'threads=1', f'maxticks={maxticks}',
           'quiet=1', 'survevery=10', f'out={out}']
    if commitk is not None:
        cmd.append(f'commitk={commitk}')
    cmd += variant.split()
    t0 = time.time()
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    m = re.search(r'GATE rt (\d+) value ([-\d.]+) Ee (-?\d+)', res)
    if not m:
        return None
    c = re.search(r'COMMIT \d+ inputs rt (\d+) ref \d+ \(teero (-?\d+)\) -> (\S+)', res)
    return {'rt': int(m.group(1)), 'ee': int(m.group(3)), 'full': out + '0.txt', 'commit': c.group(3) if c else None,
            'commit_rt': int(c.group(1)) if c else None, 'commit_k': int(c.group(2)) if c else None, 'variant': variant,
            'sec': time.time() - t0}


def main():
    stack = [sys.argv[1]]
    step = 0
    # resume: continue from the newest commit of an earlier (interrupted) run
    done = sorted((int(f[1:-4]), f) for f in os.listdir(D) if re.fullmatch(r'c\d+\.txt', f))
    for n, f in done:
        stack.append(f'{D}/{f}')
        step = n
    if done:
        log(f'resuming after step {step} from {stack[-1]}')
    wide = False
    fails = 0
    while True:
        prefix = stack[-1]
        rt0, k0 = start_k(prefix)
        if k0 >= STOP_K:
            log(f'stop: reached k {k0} (rt {rt0}) >= stopk {STOP_K} at {prefix}')
            break
        final = k0 + WINDOW >= FINISH_K - 5
        gate = 'finish' if final else str(k0 + WINDOW)
        commitk = None if final else k0 + COMMIT
        maxticks = (FINISH_K - k0 if final else WINDOW) * 2 + 60
        variants = (WIDE_R if args.get('vset') in ('r', 'r2', 'r3') else WIDE) if wide else (POST if k0 >= GREN_K else VARIANTS)
        variants = [variants[int(i)] for i in args['vidx'].split(',')] if args.get('vidx') and not wide else variants[:NVAR]
        step += 1
        with ThreadPoolExecutor(len(variants)) as ex:
            res = list(ex.map(lambda iv: run_variant(prefix, gate, commitk, iv[1], f's{step}v{iv[0]}', maxticks), enumerate(variants)))
        ok = [r for r in res if r]
        summary = ', '.join(f"v{i}:{r['rt']}/{r['ee']}" if r else f"v{i}:NONE" for i, r in enumerate(res))
        if not ok:
            log(f'step {step}: from rt {rt0} k {k0} gate {gate}: all variants failed ({summary}); backing off')
            fails += 1
            if fails >= 4:
                log('giving up after 4 failures in a row')
                break
            if len(stack) > 1 and wide:
                stack.pop()
            wide = True
            continue
        fails = 0
        lam = SEL_LAMBDA if k0 < GREN_K else float(args.get('selpost', 0.005))
        best = min(ok, key=lambda r: (r['rt'] - (0 if final else lam * r['ee']), r['rt']))
        wide = False
        if final:
            log(f'step {step}: FINISH rt {best["rt"]} ({best["rt"] / 50:.2f} s) [{best["variant"]}] ({summary})')
            with open(f'{D}/final.txt', 'w') as f:
                f.write(open(best['full']).read())
            break
        log(f'step {step}: from rt {rt0} k {k0} gate {gate}: best rt {best["rt"]} (lead {int(gate) - best["rt"]:+d}) [{best["variant"]}] '
            f'commit rt {best["commit_rt"]} k {best["commit_k"]} ({summary}, {best["sec"]:.0f}s)')
        dst = f'{D}/c{step}.txt'
        with open(dst, 'w') as f:
            f.write(open(best['commit']).read())
        stack.append(dst)


if __name__ == '__main__':
    main()
