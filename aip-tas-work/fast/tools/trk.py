#!/usr/bin/env python3
"""Track-then-splice: follow another player's trajectory tick by tick (ddsearch track=), then finish to the pickup
guided by our best run.

usage: trk.py BEST DIR TRACK [iters=N] [cutmin= cutmax=] [endmin= endmax=] [cores=4] [abeams=a,b] [bbeams=a,b]
                             [offs=o1,o2] [aims=FILE]

stage A: ddsearch best=BEST cut=c track=TRACK trackend=h trackoff=o dumpk=cores (all cores)
stage B: from each dump, ddsearch prefix=dump ref=<BEST's track aligned at the dump end> to the pickup (one core each)
Verified earlier pickups replace DIR/best.txt (and DIR/best_<rt>.txt).
"""
import os
import random
import re
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.environ.get('TRK_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
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


def aligned_ref(gtrack, rt, x, y, path):
	cand = [p for p in gtrack if abs(p[0] - rt) <= 40]
	gt = min(cand, key=lambda p: (p[1] - x) ** 2 + (p[2] - y) ** 2)
	shift = rt - gt[0]
	with open(path, 'w') as f:
		for t, gx, gy in gtrack:
			f.write(f'{t + shift} {gx} {gy}\n')
	return shift


def main():
	best_in, d, trk = sys.argv[1:4]
	o = dict(a.split('=', 1) for a in sys.argv[4:])
	iters = int(o.get('iters', 1000))
	cores = int(o.get('cores', 4))
	abeams = [int(b) for b in o.get('abeams', '20000,40000').split(',')]
	bbeams = [int(b) for b in o.get('bbeams', '30000,50000').split(',')]
	offs = [float(b) for b in o.get('offs', '0').split(',')]
	cutmin, cutmax = int(o.get('cutmin', 20)), int(o.get('cutmax', 40))
	endmin, endmax = int(o.get('endmin', 440)), int(o.get('endmax', 480))
	aims = o.get('aims')
	os.makedirs(d, exist_ok=True)
	best = os.path.join(d, 'best.txt')
	if not os.path.exists(best):
		shutil.copy(best_in, best)
	btrack, brt = track(best)
	logf = open(os.path.join(d, 'trk.log'), 'a')

	def log(m):
		print(m, flush=True)
		logf.write(m + '\n')
		logf.flush()

	log(f'start best {brt}, track {trk}')
	rng = random.Random(int(time.time()))
	for it in range(1, iters + 1):
		cut = rng.randint(cutmin, cutmax)
		h = rng.randint(endmin, endmax)
		va = {'beam': rng.choice(abeams), 'trackoff': rng.choice(offs), 'trackdecay': rng.choice([0.8, 0.9, 0.95]),
			'trackw': rng.choice([0.25, 0.5, 1.0]), 'tracklook': rng.choice([2, 4, 6]), 'rothook': rng.choice([1, 2]),
			'survive': rng.choice([6, 8]), 'cellpos': rng.choice([2, 4, 4]), 'cellvel': rng.choice([0.5, 1]),
			'seed': rng.randrange(1, 1 << 30)}
		extra = []
		if aims:
			extra = [f'aimfile={aims}', f'aimwin={rng.choice([2, 4])}']
		dump = os.path.join(d, 'dump.txt')
		for k in range(cores):
			f = dump if k == 0 else f'{dump}.{k}'
			if os.path.exists(f):
				os.remove(f)
		t0 = time.time()
		txt = subprocess.run([TOOL, MAP, f'best={best}', f'cut={cut}', f'track={trk}', f'trackend={h}', f'dumpk={cores}', f'threads={cores}',
			f'out={dump}'] + [f'{k}={x}' for k, x in va.items()] + extra, capture_output=True, text=True).stdout
		dumps = re.findall(r'RESULT dump est ([\d.]+) at rt \d+ \(ref idx \d+\) -> (\S+)', txt)
		errs = re.findall(r'err ([\d.]+)', txt)
		log(f'it {it} A cut {cut} h {h} {" ".join(f"{k}={x}" for k, x in va.items() if k != "seed")} -> {len(dumps)} dumps, '
			f'err max {max(map(float, errs)) if errs else -1:.0f} last {errs[-1] if errs else "-"} [{time.time() - t0:.0f}s]')
		if not dumps:
			continue
		procs = []
		for w in range(cores):
			e, f = dumps[w % len(dumps)]
			tr, _ = track(f)
			last = max(tr)
			refb = os.path.join(d, f'refB_{w}.txt')
			shift = aligned_ref(btrack, last[0], last[1], last[2], refb)
			vb = {'beam': rng.choice(bbeams), 'hnow': rng.choice([300, 600, 600, 1000]), 'ge': rng.choice([0, 0.01, 0.02, 0.02]),
				'rothook': rng.choice([1, 2, 2]), 'survive': rng.choice([6, 8, 8, 12]), 'cellpos': rng.choice([4, 8, 8]),
				'cellvel': rng.choice([0.5, 1, 1, 2]), 'seed': rng.randrange(1, 1 << 30)}
			if rng.random() < 0.85:
				vb['lp'] = rng.choice([0.03, 0.05, 0.1, 0.2])
				vb['latdz'] = rng.choice([16, 24, 32])
			out = os.path.join(d, f'b_{w}.txt')
			if os.path.exists(out):
				os.remove(out)
			args = [TOOL, MAP, f'best={best}', f'prefix={f}', f'ref={refb}', 'threads=1', 'slack=0', 'abort=40', f'out={out}'] + \
				[f'{k}={x}' for k, x in vb.items()]
			procs.append((subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True), out, f, shift))
		for pr, out, f, shift in procs:
			txt = pr.communicate()[0]
			m = re.search(r'RESULT (.*)', txt)
			log(f'it {it} B from {os.path.basename(f)} (lead {-shift:+d}) -> {m.group(1) if m else "crash"}')
			if os.path.exists(out):
				nt, nrt = track(out)
				if nrt is not None and nrt < brt:
					brt = nrt
					btrack = nt
					shutil.copy(out, best)
					shutil.copy(out, os.path.join(d, f'best_{nrt}.txt'))
					log(f'*** NEW BEST {nrt} (it {it}, cut {cut}, h {h})')
	log(f'end best {brt}')


if __name__ == '__main__':
	main()
