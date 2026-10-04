#!/usr/bin/env python3
"""Small harness around the `lab` tool (single-threaded, ~15 ms per call).
run(prefix_lines, cont) -> list of dict states (one per continuation tick)."""
import os, re, subprocess, math, tempfile
HERE = os.path.dirname(os.path.abspath(__file__))
WORK = os.path.dirname(HERE)
LAB = os.path.join(WORK, '..', 'ddnet', 'build-sim', 'lab')
MAP = os.path.join(WORK, 'AiP-Gores.map')
RX = re.compile(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*?vel ([\d.-]+) ([\d.-]+) \|v\| ([\d.-]+) hook (-?\d+) jumped (\d+) grounded (\d+) reload (-?\d+) proj (\d+) frz (\d+)')
TMP = os.path.join(HERE, 'tmp'); os.makedirs(TMP, exist_ok=True)

def read_inputs(path, n=None):
    L = [l.split() for l in open(path) if l.strip()]
    L = [tuple(int(v) for v in l) for l in L]
    return L if n is None else L[:n]

def parse(out):
    S = []
    for l in out.splitlines():
        m = RX.search(l)
        if m:
            g = m.groups()
            S.append(dict(rt=int(g[0]), t=int(g[1]), x=float(g[2]), y=float(g[3]), vx=float(g[4]), vy=float(g[5]), v=float(g[6]),
                          hook=int(g[7]), jumped=int(g[8]), gr=int(g[9]), frz=int(g[12])))
    return S

_pid = os.getpid()
def run(prefix, cont, extra_pre='', extra_post=''):
    """prefix: path of a prefix input file (replayed quietly); cont: list of 7-tuples (dir jump hook fire tx ty w)."""
    fn = os.path.join(TMP, f'c{_pid}.txt')
    with open(fn, 'w') as f:
        for c in cont:
            f.write(' '.join(str(int(v)) for v in c) + '\n')
    script = f'quiet;{extra_pre};replay {prefix};loud;{extra_post};replay {fn}'
    out = subprocess.run([LAB, MAP, script], capture_output=True, text=True).stdout
    S = parse(out)
    return S[:-1] if len(S) > len(cont) else S   # last line is the final print

def runscript(script):
    out = subprocess.run([LAB, MAP, script], capture_output=True, text=True).stdout
    return parse(out)

def aim(deg, r=1000):
    return (int(round(math.cos(math.radians(deg)) * r)), int(round(math.sin(math.radians(deg)) * r)))

def inp(d, j, h, deg_or_txy, w=-1):
    if isinstance(deg_or_txy, tuple):
        tx, ty = deg_or_txy
    else:
        tx, ty = aim(deg_or_txy)
    return (d, j, h, 0, tx, ty, w)

def E(s):
    return s['vx'] ** 2 + s['vy'] ** 2 - s['y']

def fmt(s):
    return f"rt {s['rt']} pos {s['x']:.0f} {s['y']:.0f} v {s['vx']:.2f} {s['vy']:.2f} |v| {s['v']:.2f} h{s['hook']} j{s['jumped']} g{s['gr']} f{s['frz']} E {E(s):.0f}"

def q256(v):
    return round(v * 256) / 256.0

def batch(start, seqs, wait=160, chunk=400):
    """Run many input sequences from a teleported start state (hook idle).
    start = dict(x,y,vx,vy,jumped). Each experiment: park at a safe spot (unfreezes), tp, run seq loudly.
    Returns list of state lists. Only exact if the sequence's first tick does not depend on a pre-existing hook."""
    res = []
    for c0 in range(0, len(seqs), chunk):
        part = seqs[c0:c0 + chunk]
        lines = ['quiet']
        for seq in part:
            lines.append('tp 144 369 0 0')
            lines.append(f'in 0 0 0 0 0 -1 {wait}')
            if start.get('jumped', 2) & 2:
                lines.append('tp 144 300 0 0')
                lines.append('in 0 1 0 0 0 -1 1')
                lines.append('in 0 0 0 0 0 -1 1')
            lines.append(f"tp {start['x']:.0f} {start['y']:.0f} {q256(start['vx']):.8f} {q256(start['vy']):.8f}")
            lines.append('print')
            lines.append('loud')
            for c in seq:
                lines.append('in ' + ' '.join(str(int(v)) for v in c[:6]))
            lines.append('quiet')
        out = subprocess.run([LAB, MAP, '-'], input='\n'.join(lines), capture_output=True, text=True).stdout
        S = parse(out)
        i = 0
        for seq in part:
            i += 1  # the print marker
            res.append(S[i:i + len(seq)])
            i += len(seq)
    return res

def state_after(prefix_path, n=None):
    """exact state after replaying the first n lines of a prefix file"""
    if n is not None:
        fn = os.path.join(TMP, f'p{_pid}.txt')
        with open(fn, 'w') as f:
            f.writelines(open(prefix_path).readlines()[:n])
        prefix_path = fn
    S = runscript(f'quiet;replay {os.path.abspath(prefix_path)};print')
    return S[-1]

def first(S, cond):
    for i, s in enumerate(S):
        if s['frz']:
            return None
        if cond(s):
            return i
    return None
