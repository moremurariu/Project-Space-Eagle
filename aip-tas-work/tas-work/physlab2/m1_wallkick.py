# Mechanics: kick from a shot at a bare wall vs aim angle and wall distance (spawn room, bare left wall at x=64).
# Tee starts at rest at (64+d, 200), dir 0 (velocity measured against the same inputs without the shot).
# Prints: explosion step (1 = the fire step) and the velocity difference vs the no-shot baseline right after it.
import math
from xl import *
x = XL()
x.cmd(['in 0 0 0 0 0 -1 3', 'gren', 'save 9'])
def traj(d, aimdeg, fire, y=200, steps=6, dirr=0):
    tx, ty = aim(aimdeg)
    lines = ['restore 9', 'reload0', f'tp {64 + d} {y} 0 0', 'loud', f'in {dirr} 0 0 {fire} {tx} {ty}'] + [f'in {dirr} 0 0 0 0 -1'] * steps + ['quiet']
    return [parse_state(l) for l in x.cmd(lines) if l.startswith('S ')]
def kick(d, aimdeg):
    A = traj(d, aimdeg, 1); B = traj(d, aimdeg, 0)
    for k, (a, b) in enumerate(zip(A, B)):
        dv = (a['vx'] - b['vx'], a['vy'] - b['vy'])
        if math.hypot(*dv) > 0.3:
            # the step's own friction (dir 0: vx *= 0.95 in the air) is applied after the kick
            return k + 1, dv[0] / 0.95, dv[1]
    return None, 0, 0
print('wall x=64, tee (64+d, 200) at rest; columns: aim offset from straight left (deg, + = aimed down-left)')
print('entries: step:kick_x,kick_y  (kick_x corrected for the 0.95 air friction of that step)')
for d in (30, 36, 42, 48, 56, 64, 80, 100):
    row = []
    for phi in (-45, -30, -15, 0, 15, 30, 45):
        k, kx, ky = kick(d, 180 - phi)
        row.append(f'{k}:{kx:5.1f},{ky:5.1f}' if k else '      -       ')
    print(f'd={d:3d}', ' '.join(row))
