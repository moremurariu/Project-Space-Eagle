# Mechanics: hook + kick interaction (spawn room). Tee hooked to the ceiling (y=64) straight above, then a kick.
from xl import *
x = XL()
x.cmd(['in 0 0 0 0 0 -1 3', 'gren', 'save 9'])
def run(lines):
    return [parse_state(l) for l in x.cmd(lines) if l.startswith('S ')]
def show(name, S):
    print(f'{name:44s}', ' | '.join(f"({s['vx']:.2f},{s['vy']:.2f}) h{s['hook']}" for s in S))
up = aim(270); dn = aim(90); left = aim(180)
# 1) hooked to the ceiling, |v| small, kick UP from the floor (tee 34 px above the floor at y=350): does the hook still pull?
for hook in (0, 1):
    L = ['restore 9', 'reload0', 'tp 300 350 0 0']
    if hook:
        L += [f'in 0 0 1 0 {up[0]} {up[1]}'] * 6 + ['tp 300 350 0 0']   # hook reaches the ceiling (y 64) and grabs
    L += ['loud', f'in 0 0 {hook} 1 {dn[0]} {dn[1]}'] + [f'in 0 0 {hook} 0 {up[0]} {up[1]}'] * 3 + ['quiet']
    show(f'kick up (floor), hook to ceiling={hook}', run(L))
# 2) hooked to the ceiling while |v| < 15 and kicked sideways (left wall at x=64, tee at x=94): hook pull after the kick
for hook in (0, 1):
    L = ['restore 9', 'reload0', 'tp 94 250 0 0']
    if hook:
        L += [f'in 0 0 1 0 {up[0]} {up[1]}'] * 6 + ['tp 94 250 0 0']
    L += ['loud', f'in 1 0 {hook} 1 {left[0]} {left[1]}'] + [f'in 1 0 {hook} 0 {up[0]} {up[1]}'] * 3 + ['quiet']
    show(f'kick right (wall), hook to ceiling={hook}', run(L))
