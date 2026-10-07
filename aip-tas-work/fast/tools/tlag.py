import re, sys, math
# lead of run (replay dump) vs a track file "rt x y" (Teero): positive = run ahead
def load(fn):
    pts={}
    for l in open(fn):
        m=re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+)',l)
        if m: pts[int(m[1])-68]=(float(m[2]),float(m[3]))
    return pts
a=load(sys.argv[1])
tr=[tuple(map(float,l.split())) for l in open(sys.argv[2]) if l.strip()]
step=float(sys.argv[3]) if len(sys.argv)>3 else 20
nxt=0
for t,x,y in tr:
    if t<nxt: continue
    nxt=t+step
    cands=[k for k in a if abs(k-t)<60]
    if not cands: continue
    # nearest point on our polyline (interpolated)
    best=None
    for k in cands:
        if k+1 not in a: continue
        (x0,y0),(x1,y1)=a[k],a[k+1]
        dx,dy=x1-x0,y1-y0; L=dx*dx+dy*dy
        u=0 if L==0 else max(0,min(1,((x-x0)*dx+(y-y0)*dy)/L))
        d=math.hypot(x0+u*dx-x,y0+u*dy-y)
        if best is None or d<best[0]: best=(d,k+u)
    print(f'rt {t:7.1f} pos {x:7.0f} {y:6.0f}  teero lead {best[1]-t:+6.2f}  dist {best[0]:4.0f}')
