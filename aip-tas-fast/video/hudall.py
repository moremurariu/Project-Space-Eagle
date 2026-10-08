"""read the HUD of every frame: python3 hudall.py TOOLDIR VIDEO TEMPLATES OUT.csv"""
import sys, pickle, subprocess, re
import numpy as np
sys.path.insert(0, sys.argv[1])
from hudocr import *
templates = pickle.load(open(sys.argv[3], 'rb'))
W, H = 300, 320
p = subprocess.Popen(['ffmpeg', '-v', 'error', '-i', sys.argv[2], '-vf', f'crop={W}:{H}:{X0}:{Y0}', '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-'],
                     stdout=subprocess.PIPE)
out = open(sys.argv[4], 'w')
out.write('frame,px,py,vx,vy,angle,ok\n')
num = re.compile(r'^-?\d+\.\d\d$')
f = 0
bad = 0
while True:
    buf = p.stdout.read(W * H * 3)
    if len(buf) < W * H * 3:
        break
    crop = np.frombuffer(buf, np.uint8).reshape(H, W, 3)
    m = textmask(crop)
    vals = []
    for (a, b) in LINES:
        cs = chars(m[a:b, 150:])
        vals.append(''.join(classify(c, templates) for c in cs))
    fv = [to_value(v) for v in vals]
    ok = all(x is not None for x in fv)
    bad += not ok
    out.write(f'{f},' + ','.join('' if x is None else f'{x:.2f}' for x in fv) + f',{int(ok)}\n')
    f += 1
print('frames', f, 'unparsed', bad)
