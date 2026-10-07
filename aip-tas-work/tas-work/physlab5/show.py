import sys, json
from xl import *
from hc5 import *
pre = sys.argv[1]; f = sys.argv[2]
d = json.load(open(f)); cut = d['cut']
x = XL(); x.load(pre, cut + 68); x.save(0)
seq = d.get('full') or d['seq']
S = x.runmany([tuples(seq)], restore=0)[0]
a = int(sys.argv[3]) if len(sys.argv) > 3 else 0
b = int(sys.argv[4]) if len(sys.argv) > 4 else 99999
for c, s in zip(seq, S):
    if a <= s['rt'] <= b:
        print(line(c).ljust(30), '|', fmt(s), 'g', s['gren'])
