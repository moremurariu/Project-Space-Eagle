"""Teero's per-tick states (ticks.csv) on our race clock + internal velocity + per-tick velocity change analysis."""
import csv, math, sys, re
rows = list(csv.DictReader(open(sys.argv[1])))
T = [(int(r['tick']), int(r['xr']), int(r['yr']), float(r['vxd']), float(r['vy']), float(r['angle'])) for r in rows]


def ramp(v):
    val = v * 50
    return 1.0 if val < 550 else 1.0 / 1.4 ** ((val - 550) / 2000)


def unramp(vxd, vy):
    vx = vxd
    for _ in range(30):
        vx = vxd / ramp(math.hypot(vx, vy))
    return vx


# our run: race tick of each position; find his start by matching our crossing at rt 0 (x 992 y 448 in 962)
ours = {}
for l in open(sys.argv[2]):
    m = re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+) .* start (-?\d+)', l)
    if m:
        ours[int(m[1]) - int(m[6])] = (float(m[2]), float(m[3]), float(m[4]), float(m[5]))
# his start: the first tick his x passes our x at rt 0 minus the fraction
x0 = ours[0][0]
k0 = next(i for i, t in enumerate(T) if t[1] >= x0)
print('his tick at our x(rt0)=%d: %d (x %d)' % (x0, T[k0][0], T[k0][1]), file=sys.stderr)
off = int(sys.argv[3]) if len(sys.argv) > 3 else T[k0][0]
out = open(sys.argv[4] if len(sys.argv) > 4 else '/dev/stdout', 'w')
out.write('rt x y vx vy angle\n')
for t in T:
    vx = unramp(t[3], t[4])
    out.write(f'{t[0] - off} {t[1]} {t[2]} {vx:.3f} {t[4]:.3f} {t[5]:.2f}\n')
