#!/usr/bin/env python3
"""Turn one of our runs into a reference for x_tig (as if it were read from a video, all certain): a track file
(label = race tick, position after that tick's step) and an inputs CSV in the video-reader format (dir, jump presses,
cursor aim, shot presses with their aim). usage: run2ref.py RUN OUT_TRACK OUT_CSV"""
import math, subprocess, sys

run, otrk, ocsv = sys.argv[1:4]
out = subprocess.run(['../ddnet/build-sim/x_trace', 'AiP-Gores.map', run], capture_output=True, text=True).stdout
T = {}
for l in out.splitlines():
    a = l.split()
    if a and a[0] == 'T':
        T[int(a[2])] = (int(a[1]), float(a[3]), float(a[4]))
L = [l.split() for l in open(run) if l.strip()]
with open(otrk, 'w') as f:
    for rt in sorted(T):
        f.write(f'{rt} {T[rt][1]:.2f} {T[rt][2]:.2f}\n')
hdr = ('frame,video_s,s_since_start,weapon,A_left,D_right,dir,jump_arrow,aim_angle_deg,aim_tx,aim_ty,recoil_units,fire,fire_aim_deg,'
       'fire_tx,fire_ty,fire_source,A_certainty,D_certainty,jump_certainty,aim_certainty,fire_certainty,fire_aim_certainty')
with open(ocsv, 'w') as f:
    f.write(hdr + '\n')
    for rt in sorted(T):
        i = T[rt][0]  # input index of the step to this tick
        if i <= 0 or i >= len(L):
            continue
        d, j, h, fi, tx, ty = (int(x) for x in L[i][:6])
        pj, pf = int(L[i - 1][1]), int(L[i - 1][3])
        ang = math.degrees(math.atan2(ty, tx))
        jp = 1 if j and not pj else 0
        fp = 1 if fi and not pf else 0
        fa = f'{ang:.2f}' if fp else ''
        f.write(f'{rt},{rt / 50:.4f},{rt / 50:.4f},grenade,{1 if d < 0 else 0},{1 if d > 0 else 0},{d},{jp},{ang:.2f},{tx},{ty},,{fp},{fa},'
                f'{tx if fp else ""},{ty if fp else ""},run,1,1,1,1.00,{"1.00" if fp else ""},{"1.00" if fp else ""}\n')
print(otrk, ocsv, len(T))
