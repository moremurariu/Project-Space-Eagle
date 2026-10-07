# clean.py FILE KEEP_RT OUT [XEND]: keep the input lines up to rt KEEP_RT, then dir-1 lines (no hook/jump/fire) until x >= XEND
import sys
from xl import *
f, keep, out = sys.argv[1], int(sys.argv[2]), sys.argv[3]
xend = float(sys.argv[4]) if len(sys.argv) > 4 else 5900
L = open(f).readlines()[:keep + 68]
x = XL()
x.load(f, keep + 68)
s = x.state()
n = 0
while s['x'] < xend and n < 60:
    out_ = x.cmd(['loud', 'r 1 0 0 0 1000 0 3', 'quiet'])
    s = parse_state([l for l in out_ if l.startswith('S ')][0])
    L.append('1 0 0 0 1000 0 3\n'); n += 1
    if s['frz']:
        print('FROZEN at', s['rt']); break
open(out, 'w').writelines(L)
print(out, 'end rt', len(L) - 68, fmt(s))
