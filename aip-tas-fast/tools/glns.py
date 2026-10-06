#!/usr/bin/env python3
"""Guided LNS: improve a run with incumbent-kept searches guided by another (faster in places) run's track.

usage: glns.py BEST DIR GUIDE [hours=H] [workers=4] [cutmin=RT] [cutmax=RT] [beams=a,b] [threads=1]

Each job cuts DIR/best.txt at a random race tick c, writes GUIDE's trajectory shifted so that it is aligned with the
best run at c (time offset at the nearest point), and runs ddsearch best=DIR/best.txt ref=<shifted guide> cut=c with a
random variant (line-following mostly on). Verified earlier pickups replace DIR/best.txt. GUIDE is an inputs file.
"""
import os
import random
import re
import shutil
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.environ.get('GLNS_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(HERE, '..', 'kog.map')
REPLAY = os.path.join(HERE, '..', 'ddnet', 'build', 'replay')


def track(path):
	out = subprocess.run([REPLAY, MAP, path, '1'], capture_output=True, text=True).stdout
	pts = []
	start = None
	for l in out.splitlines():
		m = re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) .* start (-?\d+)', l)
		if m:
			pts.append((int(m[1]), float(m[2]), float(m[3])))
			start = int(m[4])
	g = re.search(r'GRENADE at input \d+ \(tick \d+, start \d+, race tick (\d+)\)', out)
	frozen = 'frz 1' in out.split('GRENADE')[0] if g else True
	return [(t - start, x, y) for t, x, y in pts], (int(g.group(1)) if g and not frozen else None)


def main():
	best_in, d, guide = sys.argv[1:4]
	o = dict(a.split('=', 1) for a in sys.argv[4:])
	hours = float(o.get('hours', 2))
	workers = int(o.get('workers', 4))
	threads = o.get('threads', '1')
	beams = [int(b) for b in o.get('beams', '40000,60000,100000').split(',')]
	os.makedirs(d, exist_ok=True)
	best = os.path.join(d, 'best.txt')
	if not os.path.exists(best):
		shutil.copy(best_in, best)
	# guides: "file[:cutmin:cutmax]" comma separated; a file with 3 numbers per line is a track (race_tick x y)
	guides = []
	for spec in guide.split(','):
		parts = spec.split(':')
		f = parts[0]
		first = open(f).readline().split()
		if len(first) == 3:
			gt = [tuple(float(v) for v in l.split()) for l in open(f) if l.strip()]
			gt = [(int(round(t)), x, y) for t, x, y in gt]
		else:
			gt, _ = track(f)
		lo = int(parts[1]) if len(parts) > 1 else -100
		hi = int(parts[2]) if len(parts) > 2 else 100000
		guides.append((f, gt, lo, hi))
	btrack, brt = track(best)
	cutmin = int(o.get('cutmin', 0))
	cutmax = int(o.get('cutmax', brt - 5))
	lock = threading.Lock()
	state = {'rt': brt, 'track': btrack, 'job': 0}
	logf = open(os.path.join(d, 'glns.log'), 'a')

	def log(m):
		print(m, flush=True)
		logf.write(m + '\n')
		logf.flush()

	log(f'start best {brt}, guides ' + ', '.join(f'{f} [{lo},{hi}]' for f, _, lo, hi in guides))
	end = time.time() + hours * 3600

	def worker(w):
		rng = random.Random(w * 7919 + int(time.time()))
		while time.time() < end:
			with lock:
				state['job'] += 1
				job = state['job']
				src = os.path.join(d, f'src_{w}.txt')
				shutil.copy(best, src)
				cur_rt = state['rt']
				bt = state['track']
			gname, gtrack, glo, ghi = rng.choice(guides)
			lo, hi = max(cutmin, glo), min(cutmax, cur_rt - 5, ghi)
			if lo > hi:
				continue
			cut = rng.randint(lo, hi)
			bp = [p for p in bt if p[0] == cut]
			if not bp:
				continue
			_, x, y = bp[0]
			cand = [p for p in gtrack if abs(p[0] - cut) <= 40]
			if not cand:
				continue
			gt = min(cand, key=lambda p: (p[1] - x) ** 2 + (p[2] - y) ** 2)
			shift = cut - gt[0]
			ref = os.path.join(d, f'ref_{w}.txt')
			with open(ref, 'w') as f:
				for t, gx, gy in gtrack:
					f.write(f'{t + shift} {gx} {gy}\n')
			v = {
				'beam': rng.choice(beams), 'hnow': rng.choice([300, 600, 600, 1000]), 'ge': rng.choice([0, 0.01, 0.02, 0.02]),
				'rothook': rng.choice([1, 2, 2]), 'survive': rng.choice([6, 8, 8, 12]), 'cellpos': rng.choice([4, 8, 8]),
				'cellvel': rng.choice([0.5, 1, 1, 2]), 'seed': rng.randrange(1, 1 << 30),
			}
			if rng.random() < 0.85:
				v['lp'] = rng.choice([0.03, 0.05, 0.1, 0.2])
				v['latdz'] = rng.choice([16, 24, 32])
			extra = []
			aims = os.environ.get('GLNS_AIMS')
			if aims and 'teero' in os.path.basename(gname):
				af = os.path.join(d, f'aims_{w}.txt')
				with open(af, 'w') as fa:
					for l in open(aims):
						k, a = l.split()
						fa.write(f'{int(k) + shift} {a}\n')
				extra = [f'aimfile={af}', f'aimwin={rng.choice([3, 6])}']
				if rng.random() < 0.5:
					extra.append('jw=144')
			out = os.path.join(d, f'out_{w}.txt')
			if os.path.exists(out):
				os.remove(out)
			args = [TOOL, MAP, f'best={src}', f'ref={ref}', f'cut={cut}', f'threads={threads}', f'out={out}'] + [f'{k}={x}' for k, x in v.items()] + extra
			t0 = time.time()
			txt = subprocess.run(args, capture_output=True, text=True).stdout
			m = re.search(r'RESULT (.*)', txt)
			r = m.group(1) if m else 'crash'
			with lock:
				log(f'job {job} w{w} guide {os.path.basename(gname)} cut {cut} shift {shift:+d} {" ".join(f"{k}={x}" for k, x in v.items() if k != "seed")} -> {r} [{time.time() - t0:.0f}s]')
				if os.path.exists(out):
					nt, nrt = track(out)
					if nrt is not None and nrt < state['rt']:
						state['rt'] = nrt
						state['track'] = nt
						shutil.copy(out, best)
						shutil.copy(out, os.path.join(d, f'best_{nrt}.txt'))
						log(f'*** NEW BEST {nrt} (job {job}, cut {cut})')

	ths = [threading.Thread(target=worker, args=(w,)) for w in range(workers)]
	for t in ths:
		t.start()
	for t in ths:
		t.join()
	log(f'end best {state["rt"]}')


if __name__ == '__main__':
	main()
