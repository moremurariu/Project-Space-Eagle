"""grenade flight on map.txt: gsim(p, aim) -> (tick, explosion point) like DDNet (20 px/t, +0.28 t^2 drop, start p + 21 dir)"""
import math
rows=open(__file__.rsplit('/',2)[0]+'/map.txt').read().splitlines()
def solid(x,y):
    tx,ty=int(x//32),int(y//32)
    if ty<0 or ty>=len(rows) or tx<0 or tx>=len(rows[0]): return True
    return rows[ty][tx]=='#'
def gsim(p,aim,maxt=100):
    l=math.hypot(*aim); d=(aim[0]/l,aim[1]/l)
    s=(p[0]+d[0]*21,p[1]+d[1]*21)
    pos=lambda t:(s[0]+d[0]*20*t, s[1]+d[1]*20*t+0.28*t*t)
    prev=pos(0)
    for t in range(1,maxt+1):
        cur=pos(t)
        n=int(math.dist(prev,cur))+1
        for i in range(n+1):
            q=(prev[0]+(cur[0]-prev[0])*i/n, prev[1]+(cur[1]-prev[1])*i/n)
            if solid(*q): return t,q
        prev=cur
    return maxt,prev
