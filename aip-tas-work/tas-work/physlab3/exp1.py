import math, sys
from xl import XL, fmt
x=XL()
base='../runs/ex/e1021v1_0.txt'
L=[l.split() for l in open(base).read().strip().split('\n')]
def line(rt): return L[rt+67]   # input line producing state rt
x.load(base, 1149+68); x.save(0)
def run(mods, until=1185, show=True):
    x.restore(0)
    seq=[]
    for rt in range(1150, until+1):
        p=list(map(int,line(rt)[:6])) if rt+67 < len(L) else [-1,0,0,0,0,1]
        p[3]=0
        if rt in mods: p=mods[rt]
        seq.append(tuple(p))
    S=x.run(seq)
    if show:
        for s in S: print(fmt(s))
    return S
deg=float(sys.argv[1]) if len(sys.argv)>1 else 22
a=(int(1000*math.cos(math.radians(deg))), int(1000*math.sin(math.radians(deg))))
b=line(1150)
mods={1150:(int(b[0]),0,int(b[2]),1,a[0],a[1])}
x.restore(0)
run(mods, 1175)
