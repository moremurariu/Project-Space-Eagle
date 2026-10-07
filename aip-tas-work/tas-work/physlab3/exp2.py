import sys
from ex import Ex, aim
from xl import fmt
e=Ex()
b=e.line(1150)
for deg in [16,18,20,22,24,26,28]:
    a=aim(deg)
    S=e.run({1150:(b[0],0,b[2],1,a[0],a[1])}, 1170)
    # find kick
    for p,q in zip(S,S[1:]):
        dv=(q['vx']-p['vx'], q['vy']-p['vy']-0.5)
        if abs(dv[0])+abs(dv[1])>3 and q['proj']==0 and p['proj']==1:
            print(deg, 'kick at', q['rt'], 'before', fmt(p), 'after v %.2f %.2f'%(q['vx'],q['vy']))
