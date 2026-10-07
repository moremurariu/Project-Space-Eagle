#!/usr/bin/env python3
"""Python driver for physlab2/xlab (persistent process, save/restore slots).
x = XL(); x.load(path, n); x.save(0); x.restore(0); S = x.run(seq) -> list of states (dict)
seq items: (dir, jump, hook, fire, tx, ty) or (dir, jump, hook, fire, deg) with deg a float.
"""
import math
import os
import subprocess
import threading

HERE = os.path.dirname(os.path.abspath(__file__))
WORK = os.path.dirname(HERE)
XLAB = os.path.join(HERE, 'xlab')
MAP = os.path.join(WORK, 'AiP-Gores.map')
PRE = os.path.join(WORK, 'kog_pregren_978.txt')

KEYS = ('rt', 'x', 'y', 'vx', 'vy', 'hook', 'hx', 'hy', 'jumped', 'gr', 'reload', 'proj', 'frz', 'gren')


def parse_state(l):
    p = l.split()
    v = [int(p[1]), float(p[2]), float(p[3]), float(p[4]), float(p[5]), int(p[6]), float(p[7]), float(p[8]), int(p[9]), int(p[10]),
         int(p[11]), int(p[12]), int(p[13]), int(p[14])]
    return dict(zip(KEYS, v))


def aim(deg, r=1000):
    return (int(round(math.cos(math.radians(deg)) * r)), int(round(math.sin(math.radians(deg)) * r)))


def fmt_in(c):
    if len(c) == 5:
        tx, ty = aim(c[4])
        return f'in {c[0]} {c[1]} {c[2]} {c[3]} {tx} {ty}'
    return 'in ' + ' '.join(str(int(v)) for v in c[:6])


def line_of(c, w=3):
    """replay-file line for an input tuple"""
    if len(c) == 5:
        tx, ty = aim(c[4])
    else:
        tx, ty = c[4], c[5]
    return f'{c[0]} {c[1]} {c[2]} {c[3]} {tx} {ty} {w}'


class XL:
    def __init__(self):
        self.p = subprocess.Popen([XLAB, MAP], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1 << 16)

    def cmd(self, lines):
        """send lines, return output lines up to the sync marker"""
        data = '\n'.join(lines) + '\nsync\n'

        def writer():
            self.p.stdin.write(data)
            self.p.stdin.flush()
        th = threading.Thread(target=writer)
        th.start()
        out = []
        while True:
            l = self.p.stdout.readline()
            if not l:
                raise RuntimeError('xlab died')
            l = l.rstrip('\n')
            if l == 'OK':
                th.join()
                return out
            out.append(l)

    def load(self, path=PRE, n=None):
        self.cmd([f'load {path}' + (f' {n}' if n is not None else '')])

    def save(self, k=0):
        self.cmd([f'save {k}'])

    def restore(self, k=0):
        self.cmd([f'restore {k}'])

    def state(self):
        return parse_state(self.cmd(['p'])[0])

    def run(self, seq, restore=None, loud=True):
        lines = []
        if restore is not None:
            lines.append(f'restore {restore}')
        lines.append('loud' if loud else 'quiet')
        lines += [fmt_in(c) for c in seq]
        lines.append('quiet')
        if not loud:
            lines.append('p')
        out = self.cmd(lines)
        return [parse_state(l) for l in out if l.startswith('S ')]

    def runmany(self, seqs, restore=0, chunk=200):
        """run many sequences from slot `restore`; returns list of state lists"""
        res = []
        for c0 in range(0, len(seqs), chunk):
            lines = []
            for seq in seqs[c0:c0 + chunk]:
                lines.append(f'restore {restore}')
                lines.append('loud')
                lines += [fmt_in(c) for c in seq]
                lines.append('quiet')
                lines.append('p')  # marker (repeats the last state)
            out = [parse_state(l) for l in self.cmd(lines) if l.startswith('S ')]
            i = 0
            for seq in seqs[c0:c0 + chunk]:
                res.append(out[i:i + len(seq)])
                i += len(seq) + 1
        return res

    def explosions(self):
        out = self.cmd(['exp'])
        return [(int(l.split()[1]), float(l.split()[2]), float(l.split()[3])) for l in out if l.startswith('E ')]

    def close(self):
        self.p.stdin.close()
        self.p.wait()


def fmt(s):
    return (f"rt {s['rt']} pos {s['x']:.0f} {s['y']:.0f} v {s['vx']:.2f} {s['vy']:.2f} |v| {math.hypot(s['vx'], s['vy']):.2f} "
            f"h{s['hook']} j{s['jumped']} g{s['gr']} rl{s['reload']} pr{s['proj']} f{s['frz']}")


def alive(S):
    return all(not s['frz'] for s in S)


def write_run(path, prefix_path, seq, n=None):
    """full input file: prefix (first n lines) + seq lines (weapon 3)"""
    L = open(prefix_path).readlines()
    if n is not None:
        L = L[:n]
    with open(path, 'w') as f:
        f.writelines(L)
        for c in seq:
            f.write(line_of(c) + '\n')
