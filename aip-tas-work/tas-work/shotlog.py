#!/usr/bin/env python3
"""Shots of a run: fire tick/pos/aim, explosion tick, tee pos and d(v^2) at the explosion.  usage: shotlog.py INPUTS [from_rt] [to_rt]"""
import re,subprocess,math,sys,os
os.chdir(os.path.dirname(os.path.abspath(__file__)))
r0=int(sys.argv[2]) if len(sys.argv)>2 else 1030; r1=int(sys.argv[3]) if len(sys.argv)>3 else 99999
out=subprocess.run(['../ddnet/build-sim/lab','AiP-Gores.map','replay '+sys.argv[1]],capture_output=True,text=True).stdout
R=[]
for l in out.splitlines():
    m=re.match(r'in (-?\d+) (\d) (\d) (\d) (-?\d+) (-?\d+) \| rt=(-?\d+) .*pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+) .*hook (-?\d+) .*reload (\d+) proj (\d+)',l)
    if m: g=m.groups(); R.append(dict(f=int(g[3]),tx=int(g[4]),ty=int(g[5]),rt=int(g[6]),x=float(g[7]),y=float(g[8]),vx=float(g[9]),vy=float(g[10]),rl=int(g[12]),pr=int(g[13])))
for i,(p,q) in enumerate(zip(R,R[1:])):
    if q['rt']<r0 or q['rt']>r1: continue
    if q['f'] and q['rl']>p['rl']:
        print(f"FIRE rt {q['rt']} from {p['x']:.0f},{p['y']:.0f} |v| {math.hypot(p['vx'],p['vy']):.1f} aim {math.degrees(math.atan2(q['ty'],q['tx'])):.1f}")
    if q['pr']<p['pr'] or (q['f'] and q['rl']>p['rl'] and q['pr']<=p['pr']):
        d2=(q['vx']**2+q['vy']**2)-(p['vx']**2+p['vy']**2)
        print(f"   expl rt {q['rt']} tee {q['x']:.0f},{q['y']:.0f} d(v2) {d2:+.0f} |v| {math.hypot(p['vx'],p['vy']):.1f}->{math.hypot(q['vx'],q['vy']):.1f}")
