import sys
# mklob.py RUN STRIP_RT T0 TX TY OUT: RUN's lines up to race tick T0 with shots from STRIP_RT on removed, line T0 fires (TX,TY)
run, strip, t0, tx, ty, out = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], sys.argv[5], sys.argv[6]
L = [l.split() for l in open(run) if l.strip()]
res = []
for n in range(1, t0 + 68 + 1):
    f = L[n - 1][:]
    rt = n - 68
    if rt >= strip: f[3] = '0'
    if rt == t0: f[3] = '1'; f[4] = tx; f[5] = ty; f[6] = '3'
    res.append(' '.join(f))
open(out, 'w').write('\n'.join(res) + '\n')
