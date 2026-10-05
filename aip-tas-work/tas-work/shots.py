import sys,re,subprocess,math,os
# shots.py "lab prefix" inputs.txt : per explosion, the velocity change (minus gravity) and its component along the motion
pre,f=sys.argv[1],sys.argv[2]
out=subprocess.run([os.path.join(os.path.dirname(os.path.abspath(__file__)),'../ddnet/build-sim/lab'),'AiP-Gores.map',pre+';replay '+f],capture_output=True,text=True).stdout
rows=[]
for l in out.split('\n'):
    if not l.startswith('in '): continue
    m=re.search(r'rt=(-?\d+) .*pos ([-\d.]+) ([-\d.]+) .*vel ([-\d.]+) ([-\d.]+) .*hook (-?\d+) .*grounded (\d) reload (\d+) proj (\d+)',l)
    i=l.split('|')[0].split()
    rows.append((int(m.group(1)),float(m.group(2)),float(m.group(3)),float(m.group(4)),float(m.group(5)),int(m.group(6)),int(m.group(7)),int(m.group(9)),int(i[4])))
n=0; tot=0; along=0; shots=0
for k in range(1,len(rows)):
    if rows[k][8] and not rows[k-1][8]: shots+=1
    if rows[k-1][7]>rows[k][7]:
        rt,x,y,vx,vy,hk,gr,pr,fi=rows[k]; _,_,_,px,py,_,_,_,_=rows[k-1]
        dvx=vx-px; dvy=vy-py-0.5
        mag=math.hypot(dvx,dvy); sp=math.hypot(px,py) or 1
        al=(dvx*px+dvy*py)/sp
        n+=1; tot+=mag; along+=al
        print('rt %4d tile (%5.1f,%5.1f) |v| %5.1f -> %5.1f  kick %5.1f along %+5.1f hook %d' % (rt,x/32,y/32,sp,math.hypot(vx,vy),mag,al,hk))
print('shots %d explosions %d mean kick %.2f mean along %.2f' % (shots,n,tot/max(n,1),along/max(n,1)))
