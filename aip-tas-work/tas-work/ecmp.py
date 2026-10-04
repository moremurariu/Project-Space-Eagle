#!/usr/bin/env python3
"""Energy / speed vs Teero by nearest point. usage: ecmp.py RUN [k0 k1 step]"""
import re, subprocess, math, sys, os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
T={}
for l in open('teero_track.txt'):
    k,x,y=l.split(); T[int(k)]=(float(x),float(y))
def tstate(k):
    ks=range(k-3,k+4)
    mx=sum(T[i][0] for i in ks)/7; my=sum(T[i][1] for i in ks)/7
    vx=sum((i-k)*(T[i][0]-mx) for i in ks)/28; vy=sum((i-k)*(T[i][1]-my) for i in ks)/28
    # displacement -> velocity (ramp scales x only, by |v|)
    v=math.hypot(vx,vy)
    for _ in range(40):
        r=1.4**(-(50*v-550)/2000) if v>11 else 1
        vv=math.hypot(vx/r,vy); v=v+(vv-v)*0.5
    return mx,my,vx/r,vy
out=subprocess.run(['../ddnet/build-sim/lab','AiP-Gores.map','replay '+sys.argv[1]],capture_output=True,text=True).stdout
O={}
for l in out.splitlines():
    m=re.search(r'rt=(-?\d+) t=\d+ pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)',l)
    if m and l.startswith('in '): O.setdefault(int(m.group(1)),tuple(float(m.group(i)) for i in range(2,6)))
k0,k1,st=(int(a) for a in sys.argv[2:5]) if len(sys.argv)>4 else (300,980,40)
for k in range(k0,k1+1,st):
    tx,ty,tvx,tvy=tstate(k); te=tvx*tvx+tvy*tvy-ty
    rt=min((r for r in O if abs(r-k)<45), key=lambda r:(O[r][0]-tx)**2+(O[r][1]-ty)**2)
    x,y,vx,vy=O[rt]; e=vx*vx+vy*vy-y
    print(f'k {k} Teero E {te:7.0f} |v| {math.hypot(tvx,tvy):5.1f} | ours rt {rt} (lag {rt-k:+d}) E {e:7.0f} |v| {math.hypot(vx,vy):5.1f}  dE {e-te:+6.0f}')
