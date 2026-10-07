import sys
from xl import *
x = XL()
path = sys.argv[1]; a = int(sys.argv[2]); b = int(sys.argv[3])
x.load(path, a + 68)
L = open(path).readlines()[a + 68:b + 68]
seq = []
for l in L:
    p = l.split(); seq.append(tuple(int(v) for v in p[:6]))
S = x.run(seq)
for l, s in zip(L, S):
    print(l.strip().ljust(28), '|', fmt(s), 'g', s['gren'])
