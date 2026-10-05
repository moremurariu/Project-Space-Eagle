#!/usr/bin/env python3
"""mktrack.py RUN [POST] OUT: reference track 'rt x y vx vy' from RUN (race ticks >= 0, last sample per tick); after RUN's
end the POST run (default kog_full_best.txt) supplies the samples (post-pickup climb), up to rt 1400."""
import os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
def tr(f):
    out = subprocess.run([os.path.join(HERE, '../ddnet/build-sim/lab'), os.path.join(HERE, 'AiP-Gores.map'), 'replay ' + os.path.abspath(f)], capture_output=True, text=True).stdout
    R = {}
    for l in out.splitlines():
        m = re.search(r'rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)', l)
        if m and l.startswith('in '):
            R[int(m.group(1))] = tuple(float(m.group(i)) for i in range(2, 6))
    return R
run = sys.argv[1]; post = sys.argv[2] if len(sys.argv) > 3 else 'kog_full_best.txt'; out = sys.argv[-1]
a = tr(run); b = tr(post)
rows = [(k,) + a[k] for k in sorted(a) if k >= 0]
last = rows[-1][0]
rows += [(k,) + b[k] for k in sorted(b) if last < k <= 1400]
open(out, 'w').write(''.join('%d %.1f %.1f %.4f %.4f\n' % r for r in rows))
print(len(rows), rows[0], rows[-1])
