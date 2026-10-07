import re, sys, math
def load(fn):
    pts={}
    for l in open(fn):
        m=re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+)',l)
        if m: pts[int(m[1])-68]=(float(m[2]),float(m[3]),float(m[4]),float(m[5]))
    return pts
a=load(sys.argv[1]); b=load(sys.argv[2])
step=int(sys.argv[3]) if len(sys.argv)>3 else 20
end=max(k for k in a)
prev=None
for rt in range(0,max(b)+1,step):
    if rt not in b: continue
    x,y=b[rt][:2]
    cands=[t for t in range(max(0,rt-40),min(end,rt+40)+1) if t in a]
    best=min(cands, key=lambda t:(a[t][0]-x)**2+(a[t][1]-y)**2)
    # fractional via neighbor projection
    d=math.hypot(a[best][0]-x,a[best][1]-y)
    lead=best-rt
    if lead!=prev:
        print(f'rt {rt:4d} lead {lead:+d} dist {d:4.0f} |v| {math.hypot(*b[rt][2:]):5.1f} vs ref {math.hypot(*a[best][2:]):5.1f}')
    prev=lead
