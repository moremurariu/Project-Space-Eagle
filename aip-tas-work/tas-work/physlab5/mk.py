# mk.py PREFIX JSON[:key] OUT [EXTRA_TAIL]  -> full input file (prefix lines up to the cut + seq, weapon 3), plus metrics
import sys, json
from hc5 import *
pre, jf, out = sys.argv[1], sys.argv[2], sys.argv[3]
extra = int(sys.argv[4]) if len(sys.argv) > 4 else 0
key = None
if ':' in jf:
    jf, key = jf.split(':')
d = json.load(open(jf))
if key is not None:
    seq = d[key]['seq']; cut = int(sys.argv[5]) if len(sys.argv) > 5 else 955
else:
    seq = d['seq']; cut = d['cut']
L = open(pre).readlines()[:cut + 68]
with open(out, 'w') as f:
    f.writelines(L)
    for c in seq:
        f.write(line(norm(c)) + '\n')
    for _ in range(extra):
        f.write('1 0 0 0 1000 0 3\n')
