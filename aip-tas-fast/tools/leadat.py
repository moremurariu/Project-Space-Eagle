import re, sys, math, subprocess
# leadat.py REF_INPUTS FILE... : for each FILE, lead over REF (in ticks) at FILE's last tick, at the nearest REF point (interpolated)
import os
D=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
def tr(fn):
    out=subprocess.run([D+'/ddnet/build/replay',D+'/kog.map',fn,'1'],capture_output=True,text=True).stdout
    pts={}
    for l in out.splitlines():
        m=re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+) hook \d+ jumped (\d+) frz (\d) start (-?\d+)',l)
        if m: pts[int(m[1])]=(float(m[2]),float(m[3]),float(m[4]),float(m[5]),int(m[6]),int(m[7]),int(m[8]))
    st=[p[6] for p in pts.values()][-1]
    return {t-st:p for t,p in pts.items()}
ref=tr(sys.argv[1])
for fn in [f for f in sys.argv[2:] if __import__("os").path.exists(f)]:
    a=tr(fn); rt=max(a); x,y,vx,vy,j,f,_=a[rt]
    best=None
    for k in ref:
        if k+1 not in ref or abs(k-rt)>60: continue
        (x0,y0),(x1,y1)=ref[k][:2],ref[k+1][:2]
        dx,dy=x1-x0,y1-y0; L=dx*dx+dy*dy
        u=0 if L==0 else max(0,min(1,((x-x0)*dx+(y-y0)*dy)/L))
        d=math.hypot(x0+u*dx-x,y0+u*dy-y)
        if best is None or d<best[0]: best=(d,k+u)
    k=int(best[1]); rv=ref[k]
    print(f'{fn:28s} rt {rt} lead {best[1]-rt:+6.2f} dist {best[0]:4.0f} |v| {math.hypot(vx,vy):5.1f} (ref {math.hypot(rv[2],rv[3]):5.1f}) jumped {j} (ref {rv[4]}) frz {f}')
