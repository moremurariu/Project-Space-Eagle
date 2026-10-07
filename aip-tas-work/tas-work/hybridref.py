#!/usr/bin/env python3
"""Splice two x_tig references into one continuous reference: the first (e.g. Teero's video run) up to label SPLICE,
then the second (one of our runs, via run2ref.py) relabelled label = rt - SHIFT from rt SPLICE + SHIFT on. Use where
both paths meet and the second keeps the first's pace afterwards.
usage: hybridref.py TRACK1 CSV1 TRACK2 CSV2 SPLICE SHIFT OUT_TRACK OUT_CSV"""
import sys

t1, c1, t2, c2, splice, shift, ot, oc = sys.argv[1:9]
splice, shift = int(splice), int(shift)
with open(ot, 'w') as f:
    for l in open(t1):
        a = l.split()
        if len(a) >= 3 and int(a[0]) < splice:
            f.write(l if l.endswith('\n') else l + '\n')
    for l in open(t2):
        a = l.split()
        if len(a) >= 3 and int(a[0]) - shift >= splice:
            f.write(f'{int(a[0]) - shift} {a[1]} {a[2]}\n')
with open(oc, 'w') as f:
    L1 = open(c1).read().splitlines()
    hdr = L1[0].split(',')
    iS = hdr.index('s_since_start')
    f.write(L1[0] + '\n')
    for l in L1[1:]:
        v = l.split(',')
        if len(v) > iS and v[iS] and float(v[iS]) * 50 < splice - 0.5:
            f.write(l + '\n')
    L2 = open(c2).read().splitlines()
    h2 = L2[0].split(',')
    j1, j2 = h2.index('video_s'), h2.index('s_since_start')
    for l in L2[1:]:
        v = l.split(',')
        rt = float(v[j2]) * 50
        if rt - shift >= splice - 0.5:
            v[j1] = v[j2] = f'{(rt - shift) / 50:.4f}'
            f.write(','.join(v) + '\n')
print(ot, oc)
