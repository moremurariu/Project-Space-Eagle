import re, sys, subprocess, math
MAP='../kog.map'
def replay(fn):
    out=subprocess.run(['../ddnet/build/replay',MAP,fn,'1'],capture_output=True,text=True).stdout
    pts={}
    for l in out.splitlines():
        m=re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+) hook (-?\d+) jumped (\d+) frz (\d)',l)
        if m: pts[int(m[1])]=(float(m[2]),float(m[3]),float(m[4]),float(m[5]),int(m[6]),int(m[7]),int(m[8]))
    g=re.search(r'GRENADE at input (\d+) \(tick \d+, start \d+, race tick (\d+)\)',out)
    frz=min([t for t,p in pts.items() if p[6]] or [None]) if any(p[6] for p in pts.values()) else None
    return pts,(int(g.group(2)) if g else None),frz
pre=open(sys.argv[1]).read().split('\n'); pre=[l for l in pre if l.strip()]
base=[l for l in open(sys.argv[2]).read().split('\n') if l.strip()]
pp,_,_=replay(sys.argv[1]); bp,brt,_=replay(sys.argv[2])
end=len(pre); e=pp[end]
# candidate match indices in base: nearest by position+velocity
cands=sorted(bp, key=lambda t:(bp[t][0]-e[0])**2+(bp[t][1]-e[1])**2+100*((bp[t][2]-e[2])**2+(bp[t][3]-e[3])**2))[:6]
print('prefix end input %d pos %.0f %.0f vel %.2f %.2f hook %d jumped %d'%(end,*e[:4],e[4],e[5]))
for j in sorted(cands):
    b=bp[j]
    f='tp_try.txt'
    open(f,'w').write('\n'.join(pre+base[j:])+'\n')
    p,rt,frz=replay(f)
    print('  base input %d (shift %+d) pos %.0f %.0f vel %.2f %.2f hook %d jumped %d -> pickup %s frozen at %s'%(j,j-end,*b[:4],b[4],b[5],rt,frz))
