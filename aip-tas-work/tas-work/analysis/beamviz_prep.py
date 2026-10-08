# beamviz_prep.py: x_ds beamdump files (free.dump / track.dump) + traces -> data.js for the "Beam at U-turn 1" page.
# The scratch paths (D) are where the dumps were written; edit them to rerun.
import json, math, gzip, base64, struct, sys
D='/tmp/claude-0/-home-user-Project-Space-Eagle/bf839c41-4cac-52da-8623-74ac5407b85c/scratchpad/viz/'
TW='/home/user/Project-Space-Eagle/aip-tas-work/tas-work/'
def trace(f):
    T={}
    for l in open(D+f+'.tr'):
        a=l.split(); T[int(a[0])]=dict(x=float(a[1]),y=float(a[2]),vx=float(a[3]),vy=float(a[4]),hs=int(a[5]),hx=float(a[6]),hy=float(a[7]))
    return T
INC=trace('inc')
rts=sorted(r for r in INC if 1100<=r<=1260)
P=[(r,INC[r]['x'],INC[r]['y']) for r in rts]
def lead(x,y,rt):
    best=None
    for i in range(len(P)-1):
        r,ax,ay=P[i]; _,bx,by=P[i+1]
        dx,dy=bx-ax,by-ay; L=dx*dx+dy*dy or 1e-9
        u=max(0,min(1,((x-ax)*dx+(y-ay)*dy)/L))
        d=(x-ax-u*dx)**2+(y-ay-u*dy)**2
        if best is None or d<best[0]: best=(d,r+u)
    return best[1]-rt
def load(name):
    steps=[]; cur=None; root=None
    for l in open(D+name+'.dump'):
        a=l.split()
        if a[0]=='R': root=(int(a[1]),float(a[2]),float(a[3])); continue
        if a[0]=='S':
            cur=dict(step=int(a[1]),rt=int(a[2]),cand=int(a[3]),sel=int(a[4]),kept=int(a[5]),st=[]); steps.append(cur); continue
        node,par=int(a[0]),int(a[1]); x,y=float(a[2]),float(a[3]); vx,vy=float(a[4]),float(a[5]); hs=int(a[6]); hx,hy=float(a[7]),float(a[8]); rk=int(a[10])
        cur['st'].append((node,par,x,y,vx,vy,hs,hx,hy,rk))
    return root,steps
out={}
for name in ('free','track'):
    root,steps=load(name)
    prev={}  # node -> index in previous step
    arr=[]; meta=[]
    for si,s in enumerate(steps):
        idx={st[0]:i for i,st in enumerate(s['st'])}
        off=len(arr)//8
        # cheap lead: project onto the incumbent path near the right time window
        for st in s['st']:
            node,par,x,y,vx,vy,hs,hx,hy,rk=st
            pi=prev.get(par,-1) if si>0 else -1
            ld=lead(x,y,s['rt'])
            hk=1 if hs==5 else (2 if hs==4 else 0)
            arr+= [int(round(x)),int(round(y)),pi,hk|(min(rk,3)<<2),int(round(hx)) if hk else 0,int(round(hy)) if hk else 0,int(round(max(-300,min(300,ld*10)))),int(round(min(1000,math.hypot(vx,vy)*10)))]
        meta.append([s['rt'],s['cand'],s['sel'],len(s['st']),off])
        prev=idx
    raw=struct.pack('<%dh'%len(arr),*arr)
    out[name]=dict(root=root,steps=meta,data=base64.b64encode(gzip.compress(raw,9)).decode(),n=len(arr)//8)
    print(name,len(meta),'steps',len(arr)//8,'states raw',len(raw),'gz b64',len(out[name]['data']),file=sys.stderr)
# lines: incumbent (our run) and each search's own output, rt 1112..1225
def line(T,a,b): return [[r,round(T[r]['x']),round(T[r]['y']),round(math.hypot(T[r]['vx'],T[r]['vy']),1),T[r]['hs'],round(T[r]['hx']),round(T[r]['hy'])] for r in range(a,b+1) if r in T]
out['inc']=line(INC,1112,1240)
out['freeLine']=line(trace('free'),1118,1222)
out['trackLine']=line(trace('track'),1118,1203)
# tiles of the region
x0,x1,y0,y1=7300,9600,1560,2420
rows=open(TW+'map.txt').read().splitlines()
tx0,tx1,ty0,ty1=x0//32,x1//32+1,y0//32,y1//32+1
out['tiles']=dict(tx0=tx0,ty0=ty0,rows=[rows[ty][tx0:tx1] for ty in range(ty0,ty1)])
out['box']=[x0,y0,x1,y1]
open(D+'data.js','w').write('const BEAM='+json.dumps(out,separators=(',',':'))+';\n')
print('data.js',len(open(D+'data.js').read()),file=sys.stderr)
