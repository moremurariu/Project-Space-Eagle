# map Teero track ticks to the nearest point of another track (by position, searching forward)
import sys
def load(p):
    r=[]
    for l in open(p):
        a=l.split()
        if len(a)>=3 and int(a[0])>=0: r.append((int(a[0]),float(a[1]),float(a[2])))
    return r
t=load('teero_track.txt'); o=load(sys.argv[1])
td={k:(x,y) for k,x,y in t}
j=0; out=[]
for k in [int(v) for v in sys.argv[2].split(',')]:
    x,y=td[k]
    best=None
    for i in range(j,len(o)):
        d=(o[i][1]-x)**2+(o[i][2]-y)**2
        if best is None or d<best[0]: best=(d,i)
    # first local minimum after j within 60 px
    j=best[1]; out.append(o[j][0]); print(k,'->',o[j][0],'dist %.0f'%best[0]**0.5,file=sys.stderr)
print(','.join(map(str,out)))
