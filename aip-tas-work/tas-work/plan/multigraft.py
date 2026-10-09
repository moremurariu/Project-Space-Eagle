#!/usr/bin/env python3
"""multigraft.py LINE RUN OUTDIR RT0 RT1 [off=6] [spacing=5] [par=2]: x_graft LINE onto RUN at every tick in RT0..RT1
where LINE is on RUN's line (within off px) and >= 1 tick ahead (projection), D = whole ticks ahead and one less,
largest D first. Prints each graft's simulated finish (server-check the best with srvfin.sh). x_graft beam 5000
(MG_BEAM=): finds all five recent grafts (2554-2549) that beam 20000 found, 3.5-4x faster."""
# try x_graft of a line onto the run at many on-line cuts (lead >= 1, off < OFF px), latest / largest D first
import math, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
TW=os.path.dirname(os.path.dirname(os.path.abspath(__file__))); os.chdir(TW)
BIN='../ddnet/build-sim/'; MAP='AiP-Gores.map'
line, run, D0, r0, r1 = sys.argv[1], os.path.abspath(sys.argv[2]), sys.argv[3], int(sys.argv[4]), int(sys.argv[5])
OFF=float(sys.argv[6]) if len(sys.argv)>6 else 6; SP=int(sys.argv[7]) if len(sys.argv)>7 else 5; PAR=int(sys.argv[8]) if len(sys.argv)>8 else 2
os.makedirs(D0, exist_ok=True)
def traj(p, rt0):
    T={}
    for l in subprocess.run([BIN+'x_trace',MAP,p,str(rt0)],capture_output=True,text=True).stdout.splitlines():
        a=l.split()
        if len(a)>4 and a[0]=='T': T[int(a[2])]=(float(a[3]),float(a[4]))
    return T
A, B = traj(line, r0-40), traj(run, r0-40)
res, m = {}, None
for t in range(r0, r1+1):
    if t not in A: break
    p, best = A[t], None
    for r in (range(t-30,t+30) if m is None else range(int(m)-8,int(m)+12)):
        if r in B and r+1 in B:
            a,b=B[r],B[r+1]; dx,dy=b[0]-a[0],b[1]-a[1]
            u=max(0.0,min(1.0,((p[0]-a[0])*dx+(p[1]-a[1])*dy)/(dx*dx+dy*dy or 1e-9)))
            dd=math.hypot(p[0]-a[0]-u*dx,p[1]-a[1]-u*dy)
            if best is None or dd<best[0]: best=(dd,r+u)
    if best is None: break
    m=best[1]; res[t]=(m-t,best[0])
cuts=[]
for c in sorted(res, reverse=True):
    l,d=res[c]
    if l>=1.0 and d<OFF and all(abs(c-c2)>=SP for c2 in cuts): cuts.append(c)
jobs=[]
for c in cuts:
    for D in sorted({int(res[c][0]), int(res[c][0])-1}-{0}, reverse=True):
        jobs.append((c,D))
jobs.sort(key=lambda j:(-j[1],-j[0]))
print('cuts', [(c, round(res[c][0],2), round(res[c][1],1)) for c in cuts], flush=True)
def g(j):
    c,D=j; out=f'{D0}/g_c{c}_D{D}.txt'
    r=subprocess.run([BIN+'x_graft',MAP,f'prefix={os.path.abspath(line)}',f'cut={c}',f'run={run}',f'D={D}','horizon=45','beam='+os.environ.get('MG_BEAM','5000'),'threads=2','test=300',f'out={out}'],capture_output=True,text=True).stdout
    mm=re.search(r'RESULT graft finish (\d+)',r)
    return (c,D,int(mm.group(1)) if mm else None,out)
with ThreadPoolExecutor(PAR) as ex:
    for c,D,f,out in ex.map(g, jobs):
        print(f'cut {c} D {D}: {f}', flush=True)
