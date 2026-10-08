import sys, pickle
import numpy as np
from PIL import Image
sys.path.insert(0, sys.argv[1])
from hudocr import *
known = {250: ['64.04', '15.93', '34.93', '21.16', '320.42'], 300: ['97.69', '11.41', '41.56', '-7.98', '320.00'],
         420: ['185.41', '22.53', '46.33', '-16.41', '282.17']}
templates = {}
for f, vals in known.items():
    fr = np.asarray(Image.open(f'{sys.argv[2]}/f{f}.png').convert('RGB'))
    m = textmask(fr[Y0:Y0 + 320, X0:X0 + 300])
    for (a, b), v in zip(LINES, vals):
        cs = chars(m[a:b, 150:])
        digits = [c for c in cs if not ((c[3] - c[2]) <= 5)]
        want = [ch for ch in v if ch.isdigit()]
        if len(digits) != len(want):
            print('mismatch', f, v, len(digits), [(c[0], c[1], c[2], c[3]) for c in cs])
            continue
        for c, ch in zip(digits, want):
            templates.setdefault(ch, []).append(norm(c[4]))
print({k: len(v) for k, v in sorted(templates.items())})
pickle.dump(templates, open(sys.argv[3], 'wb'))
for f, vals in known.items():
    fr = np.asarray(Image.open(f'{sys.argv[2]}/f{f}.png').convert('RGB'))
    print(f, read_values(fr, templates), vals)
