#!/usr/bin/env python3
"""Guided splice: two-stage searches on a run, both stages guided by another run's track.

usage: gsplice.py BEST DIR GUIDE [hours=H] [cores=4] [cutmin=RT] [cutmax=RT] [hmin=RT] [hmax=RT]

stage A: ddsearch best=BEST ref=<GUIDE aligned at the cut> cut=c dumpat=h dumpk=cores (incumbent kept)
stage B: from each dump, ddsearch prefix=dump ref=<GUIDE aligned at the dump end> to the pickup (one core each)
Verified earlier pickups replace DIR/best.txt.
"""
import os
import random
import re
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.environ.get('GS_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
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


def variant(rng):
	v = {'hnow': rng.choice([300, 600, 600, 1000]), 'ge': rng.choice([0, 0.01, 0.02, 0.02]), 'rothook': rng.choice([1, 2, 2]),
		'survive': rng.choice([6, 8, 8, 12]), 'cellpos': rng.choice([4, 8, 8]), 'cellvel': rng.choice([0.5, 1, 1, 2]),
		'seed': rng.randrange(1, 1 << 30)}
	if rng.random() < 0.85:
		v['lp'] = rng.choice([0.03, 0.05, 0.1, 0.2])
		v['latdz'] = rng.choice([16, 24, 32])
	return v


def main():
	best_in, d, guide = sys.argv[1:4]
	o = dict(a.split('=', 1) for a in sys.argv[4:])
	hours = float(o.get('hours', 3))
	cores = int(o.get('cores', 4))
	os.makedirs(d, exist_ok=True)
	best = os.path.join(d, 'best.txt')
	if not os.path.exists(best):
		shutil.copy(best_in, best)
	gtrack, _ = track(guide)
	btrack, brt = track(best)
	cutmin, cutmax = int(o.get('cutmin', 0)), int(o.get('cutmax', brt - 150))
	hmin, hmax = int(o.get('hmin', 0)), int(o.get('hmax', brt - 40))
	logf = open(os.path.join(d, 'gsplice.log'), 'a')

	def log(m):
		print(m, flush=True)
		logf.write(m + '\n')
		logf.flush()

	log(f'start best {brt}, guide {guide}')
	rng = random.Random(int(time.time()))
	end = time.time() + hours * 3600
	it = 0
	while time.time() < end:
		it += 1
		cut = rng.randint(cutmin, cutmax)
		h = rng.randint(max(hmin, cut + 80), max(max(hmin, cut + 80), min(hmax, cut + 350)))
		p = [q for q in btrack if q[0] == cut]
		if not p:
			continue
		refa = os.path.join(d, 'refA.txt')
		aligned_ref(gtrack, cut, p[0][1], p[0][2], refa)
		va = variant(rng)
		va['beam'] = rng.choice([40000, 60000])
		dump = os.path.join(d, 'dump.txt')
		for k in range(cores):
			f = dump if k == 0 else f'{dump}.{k}'
			if os.path.exists(f):
				os.remove(f)
		t0 = time.time()
		txt = subprocess.run([TOOL, MAP, f'best={best}', f'ref={refa}', f'cut={cut}', f'dumpat={h}', f'dumpk={cores}', f'threads={cores}', f'out={dump}'] +
			[f'{k}={x}' for k, x in va.items()], capture_output=True, text=True).stdout
		dumps = re.findall(r'RESULT dump est ([\d.]+) at rt \d+ \(ref idx \d+\) -> (\S+)', txt)
		log(f'it {it} A cut {cut} h {h} -> dumps {" ".join(e for e, f in dumps)} [{time.time() - t0:.0f}s]')
		if not dumps:
			continue
		procs = []
		for w in range(cores):
			e, f = dumps[w % len(dumps)]
			tr, _ = track(f)
			last = max(tr)
			refb = os.path.join(d, f'refB_{w}.txt')
			bshift = aligned_ref(gtrack, last[0], last[1], last[2], refb)
			vb = variant(rng)
			vb['beam'] = rng.choice([30000, 50000])
			out = os.path.join(d, f'b_{w}.txt')
			if os.path.exists(out):
				os.remove(out)
			args = [TOOL, MAP, f'best={best}', f'prefix={f}', f'ref={refb}', 'threads=1', 'slack=0', 'abort=40', f'out={out}'] + [f'{k}={x}' for k, x in vb.items()]
			if rng.random() < 0.8:
				args += [f'shadowfile={guide}', f'shadowshift={bshift}', f'shadow={rng.choice([2, 3])}']
			procs.append((subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True), out, f))
		for pr, out, f in procs:
			txt = pr.communicate()[0]
			m = re.search(r'RESULT (.*)', txt)
			log(f'it {it} B from {os.path.basename(f)} -> {m.group(1) if m else "crash"}')
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
