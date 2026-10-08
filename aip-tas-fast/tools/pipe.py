#!/usr/bin/env python3
"""Chained staged searches: rebuild a run section by section, keeping the best-lead dumps.

usage: pipe.py BEST PREFIX DIR STEPS [cores=4] [keep=2] [teero=TRACK] [aims=FILE]

STEPS: comma separated "end:kind[:back]" with end a race tick or 'end' (to the pickup), kind one of
  c1     Teero-guided corridor search (ref=TRACK, his aims, energy weighted)
  exit   turn exit: energy weighted time model against BEST
  follow path/velocity following of BEST with its own inputs as candidates (shadow)
  done   plain time model + shadow to the pickup (slack, keeps any complete run)
back: restart that many ticks before the previous dump's end (default 15). Leads are measured against BEST by the
nearest point of its path; the `keep` best dumps go on. Complete runs are written to DIR/run_<rt>_<n>.txt.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..')
TOOL = os.environ.get('PIPE_TOOL', os.path.join(ROOT, 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(ROOT, 'kog.map')
REPLAY = os.path.join(ROOT, 'ddnet', 'build', 'replay')
LEADAT = os.path.join(HERE, 'leadat.py')

KINDS = {
	'c1': ['ge=0.08 hnow=600 jw=144 beam=50000', 'ge=0.15 hnow=300 jw=144 beam=50000', 'ge=0.04 hnow=600 jw=144 lp=0.05 latdz=24 beam=50000',
		'ge=0.08 hnow=600 beam=80000'],
	'exit': ['ge=0.08 hnow=600 beam=40000', 'ge=0.15 hnow=300 jw=144 beam=40000', 'ge=0.08 hnow=600 jw=144 beam=60000', 'ge=0.05 hnow=600 beam=60000'],
	'follow': ['shadow=3 velw=4 latq=0.01 velsym=1 ge=0 hnow=0 beam=100000', 'shadow=2 velw=4 latq=0.01 ge=0.04 hnow=300 beam=50000',
		'shadow=2 velw=1 latq=0.003 ge=0.02 hnow=600 beam=50000', 'shadow=3 velw=10 latq=0.02 velsym=1 ge=0 hnow=0 beam=50000'],
	'done': ['shadow=2 ge=0.02 hnow=600 lp=0.05 latdz=24 beam=50000', 'shadow=3 ge=0.04 hnow=300 beam=50000', 'shadow=2 ge=0.02 hnow=600 beam=50000 seed=5',
		'shadow=2 velw=1 latq=0.003 ge=0.02 hnow=600 beam=50000'],
}


def leads(best, files):
	out = subprocess.run(['python3', LEADAT, best] + files, capture_output=True, text=True).stdout
	r = []
	for l in out.splitlines():
		m = re.match(r'(\S+)\s+rt (\d+) lead\s+([-+\d.]+) dist\s+(\d+) \|v\|\s+([\d.]+) \(ref\s+([\d.]+)\)', l)
		if m:
			r.append((float(m[3]), m[1], int(m[4]), float(m[5]), float(m[6])))
	return r


def track(path):
	out = subprocess.run([REPLAY, MAP, path, '1'], capture_output=True, text=True).stdout
	pts = []
	start = 0
	for l in out.splitlines():
		m = re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) .* start (-?\d+)', l)
		if m:
			pts.append((int(m[1]), float(m[2]), float(m[3])))
			start = int(m[4])
	return [(t - start, x, y) for t, x, y in pts]


def aligned_ref(btrack, prefix, path):
	# BEST's track shifted so that it is level with the end of PREFIX (time offset at the nearest point)
	rt, x, y = track(prefix)[-1]
	cand = [q for q in btrack if abs(q[0] - rt) <= 40]
	gt = min(cand, key=lambda q: (q[1] - x) ** 2 + (q[2] - y) ** 2)
	shift = rt - gt[0]
	with open(path, 'w') as f:
		for t, gx, gy in btrack:
			f.write(f'{t + shift} {gx} {gy}\n')
	return shift


def pickup(f):
	out = subprocess.run([REPLAY, MAP, f], capture_output=True, text=True).stdout
	m = re.search(r'race tick (\d+)\)', out)
	return int(m.group(1)) if m else None


def main():
	best, prefix, d, steps = sys.argv[1:5]
	o = dict(a.split('=', 1) for a in sys.argv[5:])
	cores = int(o.get('cores', 4))
	keep = int(o.get('keep', 2))
	teero = o.get('teero')
	aims = o.get('aims')
	distpen = float(o.get('distpen', 0.03))
	os.makedirs(d, exist_ok=True)
	logf = open(os.path.join(d, 'pipe.log'), 'a')

	def log(m):
		print(m, flush=True)
		logf.write(m + '\n')
		logf.flush()

	btrack = track(best)
	kept = [prefix]
	for si, st in enumerate(steps.split(',')):
		parts = st.split(':')
		h, kind = parts[0], parts[1]
		back = int(parts[2]) if len(parts) > 2 else 15
		cut = []
		for p in kept:
			lines = open(p).read().splitlines()
			q = os.path.join(d, f'cut{si}_{os.path.basename(p)}')
			with open(q, 'w') as f:
				f.write('\n'.join(lines[:len(lines) - back]) + '\n')
			cut.append(q)
		variants = KINDS[kind]
		jobs = []
		n = 0
		per = max(1, cores // len(cut))
		for p in cut:
			for v in variants[:per]:
				n += 1
				out = os.path.join(d, f's{si}_{n}.txt')
				for f in [out, out + '.1']:
					if os.path.exists(f):
						os.remove(f)
				args = [TOOL, MAP, f'best={best}', f'prefix={p}', 'threads=1', 'rothook=2', 'abort=100000', f'out={out}'] + v.split()
				if kind == 'c1':
					args += [f'ref={teero}'] + ([f'aimfile={aims}', 'aimwin=3'] if aims else [])
				if kind == 'exit':
					rf = os.path.join(d, f'ref{si}_{n}.txt')
					sh = aligned_ref(btrack, p, rf)
					args += [f'ref={rf}']
				if kind == 'done':
					args += ['slack=30', 'incumbent=0']
				if h != 'end':
					args += [f'dumpat={h}', 'dumpk=2']
				jobs.append((subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True), out, p, v))
		files = []
		for pr, out, p, v in jobs:
			txt = pr.communicate()[0]
			m = re.search(r'RESULT (.*)', txt)
			log(f'step {si} ({h} {kind}) from {os.path.basename(p)} [{v}] -> {m.group(1) if m else "crash"}')
			for f in [out, out + '.1']:
				if os.path.exists(f):
					files.append(f)
		if h == 'end':
			res = []
			for k, f in enumerate(files):
				rt = pickup(f)
				if rt is not None:
					q = os.path.join(d, f'run_{rt}_{k}.txt')
					subprocess.run(['cp', f, q])
					res.append((rt, q))
			for rt, q in sorted(res):
				log(f'complete: {q} pickup {rt}')
			break
		# rank by lead, penalising distance from BEST's line (a lead on another line rarely survives following)
		r = sorted(leads(best, files), key=lambda q: q[0] - distpen * max(0, q[2] - 20), reverse=True)
		for ld, f, dist, v, rv in r:
			log(f'  {os.path.basename(f)} lead {ld:+.2f} dist {dist} |v| {v} (ref {rv})')
		if not r:
			log('no dumps')
			return
		kept = [f for _, f, _, _, _ in r[:keep]]
	log('done')


if __name__ == '__main__':
	main()
