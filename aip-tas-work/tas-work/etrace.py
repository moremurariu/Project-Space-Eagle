import sys,re,subprocess
# usage: etrace.py SCRIPT  (lab script); prints tick, input, pos, vel, E=v^2-y, hook, jumped, grounded
out=subprocess.run(['../ddnet/build-sim/lab','AiP-Gores.map',sys.argv[1]],capture_output=True,text=True).stdout
for i,l in enumerate(out.splitlines()):
    m=re.search(r'(?:in ([-\d ]+) \| )?rt=(-?\d+) t=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) \|v\| ([\d.]+) hook (-?\d+) jumped (\d+) grounded (\d+)',l)
    if not m: continue
    inp,rt,t,x,y,vx,vy,v,h,j,g=m.groups()
    x,y,vx,vy=map(float,(x,y,vx,vy))
    print('%4s rt=%4s in[%-22s] pos %7.1f %6.1f vel %6.2f %6.2f |v| %5.2f E %7.1f h%s j%s g%s'%(t,rt,inp or '',x,y,vx,vy,float(v),vx*vx+vy*vy-y,h,j,g))
