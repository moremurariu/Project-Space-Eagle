# Mechanics: kick and jump in the same tick (spawn room floor y=384 at x<=640). Tee at (300, 350) (34 px above the floor).
from xl import *
x = XL()
x.cmd(['in 0 0 0 0 0 -1 3', 'gren', 'save 9'])
def run(lines):
    return [parse_state(l) for l in x.cmd(lines) if l.startswith('S ')]
def case(name, vy0, jump, fire, aimdeg=90, pre_air_jump=False, steps=2):
    tx, ty = aim(aimdeg)
    L = ['restore 9', 'reload0']
    if pre_air_jump:
        # use up the air jump for real (avoid the lab 'jumped' bug): jump in the air once, then teleport
        L += ['tp 300 250 0 0', 'in 0 1 0 0 0 -1', 'in 0 0 0 0 0 -1']
    L += [f'tp 300 350 0 {vy0}', 'loud', f'in 0 {jump} 0 {fire} {tx} {ty}'] + ['in 0 0 0 0 0 -1'] * (steps - 1) + ['quiet']
    S = run(L)
    print(f'{name:38s}', ' | '.join(f"v ({s['vx']:.2f},{s['vy']:.2f}) y {s['y']:.0f} j{s['jumped']}" for s in S))
for vy0 in (0, 6):
    print(f'-- start vy {vy0}')
    case('nothing', vy0, 0, 0)
    case('kick only (shoot down)', vy0, 0, 1)
    case('air jump only', vy0, 1, 0)
    case('air jump + kick same tick', vy0, 1, 1)
    case('kick, air jump next tick', vy0, 0, 1, steps=1)
    x.cmd([])
# kick then jump next tick, explicit
tx, ty = aim(90)
S = run(['restore 9', 'reload0', 'tp 300 350 0 6', 'loud', f'in 0 0 0 1 {tx} {ty}', 'in 0 1 0 0 0 -1', 'in 0 0 0 0 0 -1', 'quiet'])
print('kick then air jump next tick       ', ' | '.join(f"v ({s['vx']:.2f},{s['vy']:.2f}) j{s['jumped']}" for s in S))
S = run(['restore 9', 'reload0', 'tp 300 350 0 6', 'loud', 'in 0 1 0 0 0 -1', f'in 0 0 0 1 {tx} {ty}', 'in 0 0 0 0 0 -1', 'quiet'])
print('air jump then kick next tick       ', ' | '.join(f"v ({s['vx']:.2f},{s['vy']:.2f}) j{s['jumped']}" for s in S))
# sideways kick + jump: shoot at the left wall from (94, 200)
tx, ty = aim(180)
for j in (0, 1):
    S = run(['restore 9', 'reload0', 'tp 94 200 0 6', 'loud', f'in 0 {j} 0 1 {tx} {ty}', 'in 0 0 0 0 0 -1', 'quiet'])
    print(f'side kick (left wall) jump={j}         ', ' | '.join(f"v ({s['vx']:.2f},{s['vy']:.2f}) j{s['jumped']}" for s in S))
