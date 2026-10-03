import re,sys,subprocess,math
# usage: vsghost.py INPUTS [every]: race-time lead vs Teero along his line (projection with a monotone index)
T=[]
for l in open('teero_track.txt'):
    p=l.split()
    if len(p)>=3:
        try: T.append((int(p[0]),float(p[1]),float(p[2])))
        except: pass
out=subprocess.run(['../ddnet/build-sim/lab','AiP-Gores.map','replay '+sys.argv[1]],capture_output=True,text=True).stdout
pts=[]
for l in out.splitlines():
    if not l.startswith('in '): continue
    m=re.search(r'rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)',l)
    rt=int(m.group(1))
    if rt>=0: pts.append((rt,float(m.group(3)),float(m.group(4)),math.hypot(float(m.group(5)),float(m.group(6)))))
every=int(sys.argv[2]) if len(sys.argv)>2 else 100
idx=70  # Teero index of race tick 0
prev=None
for rt,x,y,v in pts:
    best=None
    for j in range(max(0,idx-20),min(len(T)-1,idx+120)):
        (k0,x0,y0),(k1,x1,y1)=T[j],T[j+1]
        dx,dy=x1-x0,y1-y0; L2=dx*dx+dy*dy
        t=max(0,min(1,((x-x0)*dx+(y-y0)*dy)/L2)) if L2>0 else 0
        d=math.hypot(x0+t*dx-x,y0+t*dy-y)
        if best is None or d<best[0]: best=(d,j,k0+t)
    d,idx,kt=best
    if rt%every==0:
        lead=kt-rt
        print(f"rt {rt:5d} pos ({x:6.0f},{y:5.0f}) |v| {v:5.1f}  teero k {kt:7.1f}  lead {lead:+6.1f}  dist {d:4.0f}" + ('' if prev is None else f"  ({lead-prev:+.1f})"))
        prev=lead
