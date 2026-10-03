#!/usr/bin/env python3
"""Grenade pickup + shaft exit: hook-only climbs with seg (fire=0), the exit stack with pf (pre-fired shot + point-blank
shot, fine aims), then seg with shots from each pf result to a later gate. A plain seg run from the prefix is the control.

usage: gren.py PREFIX NAME [gate=1100] [climbgate=1036] [workers=3]   (results in runs/gren/NAME/)
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
os.chdir(HERE)
SEG = '../ddnet/build-sim/seg'
PF = '../ddnet/build-sim/pf'
MAP = 'AiP-Gores.map'
args = dict(a.split('=', 1) for a in sys.argv[3:])
GATE = int(args.get('gate', 1100))
CLIMB = int(args.get('climbgate', 1036))
WORKERS = int(args.get('workers', 3))
PREFIX = sys.argv[1]
D = f'runs/gren/{sys.argv[2]}'
os.makedirs(D, exist_ok=True)

LAT = 'latpen=0.3 latdz=4 latk0=995 latk1=1030'
CLIMB_VARIANTS = [f'ghost=1 hnow=600 ghoste=0.02 {LAT}', f'ghost=1 hnow=300 ghoste=0 {LAT}', 'ghost=1 hnow=300 ghoste=0 latpen=1 latdz=2 latk0=990 latk1=1030',
                  'ghost=1 hnow=600 ghoste=0.02 pjc=250 pgc=250']
POST = 'ghost=1 hnow=600 ghoste=0.02 kcredit=2 kready=10'


def seg(prefix, gate, extra, out, maxticks=400):
    cmd = [SEG, MAP, f'prefix={prefix}', f'gate={gate}', 'horizon=150', 'beam=20000', 'threads=1', f'maxticks={maxticks}', 'quiet=1',
           'survevery=10', f'out={out}'] + extra.split()
    res = subprocess.run(cmd, capture_output=True, text=True).stdout
    m = re.search(r'GATE rt (\d+) value ([-\d.]+) Ee (-?\d+)', res)
    return (int(m.group(1)), int(m.group(3)), out + '0.txt') if m else None


def log(msg):
    with open(f'{D}/gren.log', 'a') as f:
        f.write(msg + '\n')
    print(msg, flush=True)


def main():
    with ThreadPoolExecutor(WORKERS) as ex:
        control = ex.submit(seg, PREFIX, GATE, POST, f'{D}/control_')
        climbs = list(ex.map(lambda iv: seg(PREFIX, CLIMB, 'fire=0 ' + iv[1], f'{D}/climb{iv[0]}_'), enumerate(CLIMB_VARIANTS)))
        cands = []
        for i, c in enumerate(climbs):
            log(f'climb {i}: {c}')
            if not c:
                continue
            out = subprocess.run([PF, MAP, f'base={c[2]}', 'rel=1', 't1lo=3', 't1hi=40', 'tend=0', 'xf=5900', 'res=0.05', 'res2=1',
                                  'keep=600', 'top=4', f'threads={WORKERS}', f'out={D}/pf{i}_'], capture_output=True, text=True).stdout
            for m in re.finditer(r'PF (\d+) score ([-\d.]+) .* -> (\S+)', out):
                log(f'  pf {i}.{m.group(1)} score {m.group(2)} -> {m.group(3)}')
                cands.append(m.group(3))
        res = list(ex.map(lambda f: (f, seg(f, GATE, POST, f[:-4] + '_g_')), cands))
        c = control.result()
        log(f'control: {c}')
        best = c
        for f, r in res:
            log(f'from {f}: {r}')
            if r and (best is None or r[0] < best[0] or (r[0] == best[0] and r[1] > best[1])):
                best = r
        log(f'BEST {best}')


if __name__ == '__main__':
    main()
