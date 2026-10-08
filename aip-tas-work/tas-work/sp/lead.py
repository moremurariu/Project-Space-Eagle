import sys
from trk import *
inc=sys.argv[1]; f=sys.argv[2]; a=int(sys.argv[3]); b=int(sys.argv[4]); st=int(sys.argv[5]) if len(sys.argv)>5 else 6
T0,E0=trace(inc); T,E=trace(f)
R={t:T0[t]['p'] for t in T0}
L=lagmap({t:T[t] for t in T if t>=a},R,a)
for t in range(a,b,st):
    if t in L:
        k=int(L[t][1])
        print(t,'lead %5.1f lat %3.0f |v| %3.0f inc|v| %3.0f'%(L[t][1]-t, L[t][0], T[t]['s'], T0[k]['s'] if k in T0 else 0), ' '.join('K%d:%.0f/%.2f'%(u,e['f'],e['cos']) for u in range(t,t+st) for e in E.get(u,[])), '|', ' '.join('k%d:%.0f/%.2f'%(u,e['f'],e['cos']) for u in range(k,k+st) for e in E0.get(u,[])))
