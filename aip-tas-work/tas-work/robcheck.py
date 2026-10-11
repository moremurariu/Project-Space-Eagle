#!/usr/bin/env python3
"""robcheck.py INPUTS [TESTRUNNER] [shift=1]: freeze margins of a run on the server replay (TasReplay.Run, TAS_TRACE=1).
For every tick's movement segment (the tee centre, previous -> current position, as the server's GetMapIndices samples
it every 1 px) report where DDNet's sampling would hit a freeze tile if the whole segment were shifted by up to
`shift` px in x and / or y. Another platform's float rounding can move the tee by a pixel (Oct 11: 2531 / 2533 / 2534
froze on the user's server at line 2167 - their x was 1 px smaller, the 1-px sample i=27 of the 2166 -> 2167 move
landed 0.6 px inside freeze tile (24, 102), which our sampling misses by 0.4 px). Exit code 1 when any tick fails."""
import math, os, subprocess, sys

TW = os.path.dirname(os.path.abspath(__file__))
inp = os.path.abspath(sys.argv[1])
tr = os.path.abspath(sys.argv[2]) if len(sys.argv) > 2 and '=' not in sys.argv[2] else os.path.join(TW, '..', 'ddnet', 'build-sim', 'testrunner')
kw = dict(a.split('=', 1) for a in sys.argv[2:] if '=' in a)
SH = int(kw.get('shift', 1))
M = [l.rstrip('\n') for l in open(os.path.join(TW, 'map.txt'))]


def frz(x, y):
    return 0 <= y < len(M) and 0 <= x < len(M[y]) and M[y][x] == 'f'


env = dict(os.environ, TAS_MAP=os.path.join(TW, 'AiP-Gores.map'), TAS_INPUTS=inp, TAS_TRACE='1')
out = subprocess.run([tr, '--gtest_filter=TasReplay.Run'], capture_output=True, text=True, env=env, cwd=os.path.dirname(tr)).stdout
P, start, fin, frozen = {}, None, None, None
for l in out.splitlines():
    a = l.split()
    if len(a) >= 6 and a[0].isdigit() and a[5].startswith('frz='):
        P[int(a[0])] = (float(a[1]), float(a[2]), a[5] == 'frz=1')
    if 'start tick' in l and 'finish tick' in l:
        w = l.split()
        start, fin = int(w[w.index('start') + 2]), int(w[w.index('finish') + 2])
    if l.startswith('frozen ticks'):
        frozen = int(l.split()[2])
print('server replay: start %s finish %s -> %s ticks, frozen ticks %s' % (start, fin, fin - start if fin and fin > 0 else None, frozen))
bad = []
for n in sorted(P):
    if n - 1 not in P or P[n - 1][2]:
        continue
    (ax, ay, _), (bx, by, _) = P[n - 1], P[n]
    d = math.hypot(bx - ax, by - ay)
    hits = []
    for sx in range(-SH, SH + 1):
        for sy in range(-SH, SH + 1):
            if d == 0:
                continue
            for i in range(int(d + 1)):
                f = i / d
                x, y = ax + sx + (bx - ax) * f, ay + sy + (by - ay) * f
                if frz(int(x) // 32, int(y) // 32):
                    hits.append((sx, sy))
                    break
    if hits:
        bad.append(n)
        print('line %d (race %s): (%g,%g) -> (%g,%g) hits freeze when shifted by %s' % (
            n + 1, n - start if start is not None else '?', ax, ay, bx, by, ' '.join('%+d%+d' % h for h in hits)))
print('%d ticks within %d px of a freeze hit' % (len(bad), SH))
sys.exit(1 if bad or not fin or fin < 0 or frozen else 0)
