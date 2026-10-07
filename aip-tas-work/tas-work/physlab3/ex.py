"""experiment helper: base line + modifications. mods: {rt: (d,j,h,f,tx,ty)} replaces the input line producing state rt;
lines after the base's end default to (-1,0,0,0,0,1)."""
import math
from xl import XL, fmt
BASE='../runs/ex/e1025v1_0.txt'
class Ex:
    def __init__(self, base=BASE, start=1149):
        self.x=XL(); self.base=base
        self.L=[list(map(int,l.split()[:6])) for l in open(base).read().strip().split('\n')]
        self.start=start
        self.x.load(base, start+68); self.x.save(0)
    def line(self, rt):
        i=rt+67
        return list(self.L[i]) if i < len(self.L) else [-1,0,0,0,0,1]
    def seq(self, mods, until, nofire=True):
        out=[]
        for rt in range(self.start+1, until+1):
            p=self.line(rt)
            if nofire: p[3]=0
            if rt in mods: p=list(mods[rt])
            out.append(tuple(p))
        return out
    def run(self, mods, until, show=False, nofire=True):
        S=self.x.run(self.seq(mods, until, nofire), restore=0)
        if show:
            for s in S: print(fmt(s))
        return S
def aim(deg, r=1000):
    return (int(round(r*math.cos(math.radians(deg)))), int(round(r*math.sin(math.radians(deg)))))
