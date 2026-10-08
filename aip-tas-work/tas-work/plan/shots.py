#!/usr/bin/env python3
"""shots.py RUN [minflight=8]: the run's real grenade shots (reload timer jumps) matched to explosions in order:
prints "fire_rt explode_rt flight" per shot with flight >= minflight (pre-fires / lobs), one per line."""
import os, subprocess, sys
TW = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def shots(run):
    out = subprocess.run([os.path.join(TW, '../ddnet/build-sim/x_trace'), os.path.join(TW, 'AiP-Gores.map'), run],
                         capture_output=True, text=True).stdout
    S, E, prl = [], [], 0
    for l in out.splitlines():
        a = l.split()
        if a[0] == 'T':
            rl = int(a[a.index('rl') + 1])
            if rl > prl: S.append(int(a[2]))
            prl = rl
        elif a[0] == 'E':
            E.append(int(a[2]))
    q, res = list(S), []
    for e in E:
        c = [f for f in q if f <= e]
        if c:
            q.remove(c[0]); res.append((c[0], e, e - c[0]))
    return res
if __name__ == '__main__':
    mf = int(sys.argv[2]) if len(sys.argv) > 2 else 8
    for f, e, d in shots(sys.argv[1]):
        if d >= mf: print(f, e, d)
