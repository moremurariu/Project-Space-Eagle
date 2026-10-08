import sys, math
from trk import *
run=sys.argv[1]; step=int(sys.argv[2]) if len(sys.argv)>2 else 10
T,E=trace(run); R=track()
L=lagmap({t:T[t] for t in T if t>=985},R,1002)
def disp(P,k,w=10):
    k=int(round(k))
    if k-w//2 in P and k+w//2 in P:
        a,b=P[k-w//2],P[k+w//2]; return math.hypot(b[0]-a[0],b[1]-a[1])/w
Ap={t:T[t]['p'] for t in T}
prev=None
for t in range(990,max(L),step):
    if t not in L: continue
    su=disp(Ap,t); sh=disp(R,L[t][1])
    if su is None or sh is None: continue
    lag=t-L[t][1]
    d=lag-prev if prev is not None else 0; prev=lag
    ks=' '.join('K%d:%.0f/%.1f'%(u,e['f'],e['cos']) for u in range(t,t+step) for e in E.get(u,[]))
    hk=sum(1 for u in range(t,t+step) if u in T and T[u]['hs']==5)
    print('%4d lag %5.1f (%+4.1f) us %5.1f him %5.1f gap %+5.1f |v| %5.1f h%d %s'%(t,lag,d,su,sh,sh-su,T[t]['s'],hk,ks))
