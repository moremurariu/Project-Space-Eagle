import sys,re,subprocess,os
# usage: trace.py "lab script prefix" inputs.txt [every] [from]
pre,f=sys.argv[1],sys.argv[2]; ev=int(sys.argv[3]) if len(sys.argv)>3 else 1; fr=int(sys.argv[4]) if len(sys.argv)>4 else 0
out=subprocess.run([os.path.join(os.path.dirname(os.path.abspath(__file__)),'../ddnet/build-sim/lab'),'AiP-Gores.map',pre+';replay '+f],capture_output=True,text=True).stdout
n=0
for l in out.split('\n'):
    if not l.startswith('in '): continue
    n+=1
    i=l.split('|')[0].split()
    d,j,h,fi,tx,ty=map(int,i[1:7])
    m=re.search(r'pos ([-\d.]+) ([-\d.]+) .*vel ([-\d.]+) ([-\d.]+) \|v\| ([\d.]+) hook (-?\d+) jumped (\d+) grounded (\d)',l)
    x,y,vx,vy,v,hs,jd,gr=m.groups()
    tag=('FIRE(%d,%d) '%(tx,ty) if fi else '')+('J ' if j else '')+('G ' if gr=='1' else '')
    if n>=fr and (n%ev==0 or fi or j or gr=='1'):
        print('%3d (%6.1f,%5.1f) v(%6.1f,%6.1f)|%5.1f| dir%2d hook%d(%s) %s'%(n,float(x)/32,float(y)/32,float(vx),float(vy),float(v),d,h,hs,tag))
