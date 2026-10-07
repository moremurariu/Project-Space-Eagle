#!/usr/bin/env python3
"""gates.py FILE...: first race tick (fractional) at x <= 9100 / 8700 / 8200 / 7700 (moving left, y<=2098 for 7700), |v| there,
freeze check, shots after rt 1110."""
import re, subprocess, sys, math, os
HERE=os.path.dirname(os.path.abspath(__file__))
def states(f):
    out=subprocess.run([HERE+'/../../ddnet/build-sim/lab', HERE+'/../AiP-Gores.map', 'replay '+f], capture_output=True, text=True).stdout
    S=[]
    for l in out.splitlines():
        m=re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) .*pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*reload (\d+) .*frz (\d)', l)
        if m:
            g=m.groups(); S.append(dict(f=int(g[3]), rt=int(g[6]), x=float(g[7]), y=float(g[8]), vx=float(g[9]), vy=float(g[10]), rl=int(g[11]), frz=int(g[12])))
    return S
for f in sys.argv[1:]:
    S=states(f)
    res=[]
    for gx,ylim in ((9100,9e9),(8700,9e9),(8200,9e9),(7700,2098)):
        r='-'
        for p,q in zip(S,S[1:]):
            if q['rt']>1100 and q['x']<=gx and q['vx']<=(-15 if gx==7700 else 0) and q['y']<=ylim and (gx>7700 or q['y']>=2040):
                fr=min(1.0,max(0.0,(p['x']-gx)/(p['x']-q['x']))) if p['x']>q['x'] else 1.0
                r='%.2f/%.1f'%(q['rt']-1+fr, math.hypot(q['vx'],q['vy'])); break
        res.append(r)
    shots=[q['rt'] for p,q in zip(S,S[1:]) if q['rt']>1110 and q['rl']>p['rl']]
    print('%-34s x<=9100 %-12s 8700 %-12s 8200 %-12s 7700(y<=2098) %-12s frz %d end rt %d shots %s'%(os.path.basename(f),*res,max(s['frz'] for s in S),S[-1]['rt'],shots))
