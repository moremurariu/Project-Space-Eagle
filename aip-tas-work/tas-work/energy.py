import sys,re,subprocess,math,os
# energy.py "lab prefix" inputs.txt [from] [to]: per-tick change of E = v^2 - y attributed to hook / jump / ground / air
pre,f=sys.argv[1],sys.argv[2]; fr=int(sys.argv[3]) if len(sys.argv)>3 else 0; to=int(sys.argv[4]) if len(sys.argv)>4 else 10**9
out=subprocess.run([os.path.join(os.path.dirname(os.path.abspath(__file__)),'../ddnet/build-sim/lab'),'AiP-Gores.map',pre+';replay '+f],capture_output=True,text=True).stdout
rows=[]
for l in out.split('\n'):
    if not l.startswith('in '): continue
    i=l.split('|')[0].split()
    m=re.search(r'rt=(-?\d+) .*pos ([-\d.]+) ([-\d.]+) .*vel ([-\d.]+) ([-\d.]+) .*hook (-?\d+) jumped (\d+) grounded (\d) reload (\d+) proj (\d+)',l)
    rt,x,y,vx,vy,hk,jd,gr,rl,pr=m.groups()
    rows.append(dict(rt=int(rt),x=float(x),y=float(y),vx=float(vx),vy=float(vy),hook=int(hk),jumped=int(jd),gr=int(gr),proj=int(pr),dir=int(i[1]),jump=int(i[2]),hin=int(i[3])))
acc={'hook':0,'jump':0,'ground':0,'air':0,'expl':0}
for k in range(1,len(rows)):
    a,b=rows[k-1],rows[k]
    if not (fr<=b['rt']<=to): continue
    Ea=a['vx']**2+a['vy']**2-a['y']; Eb=b['vx']**2+b['vy']**2-b['y']; d=Eb-Ea
    if a['proj']>b['proj']: c='expl'
    elif b['jumped']!=a['jumped'] and b['jump'] and (b['jumped']&1 or b['jumped']&2) and abs(b['vy']+12)<1.5 or abs(b['vy']+13.2)<1.5 and b['jump']: c='jump'
    elif b['gr'] or a['gr']: c='ground'
    elif b['hook']==5 and math.hypot(a['vx'],a['vy'])>14.5: c='hook'
    else: c='air'
    acc[c]+=d
print(' '.join('%s %+.0f'%(k,v) for k,v in acc.items()))
