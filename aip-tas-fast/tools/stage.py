#!/usr/bin/env python3
"""Staged follow search: carry a lead over the best run stage by stage.

usage: stage.py BEST PREFIX DIR ends=e1,e2,...[,end] [cores=4] [beam=50000] [keep=2]

For each stage end h (race tick; 'end' = to the pickup), runs `cores` ddsearch variants from each kept prefix (shadow
inputs, path/velocity following, different weights), dumps 2 states each at h, measures every dump's lead over BEST
(nearest point on BEST's path) and keeps the `keep` best for the next stage. A pickup earlier than BEST's is written
to DIR/best_<rt>.txt.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..')
TOOL = os.environ.get('STAGE_TOOL', os.path.join(ROOT, 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(ROOT, 'kog.map')
REPLAY = os.path.join(ROOT, 'ddnet', 'build', 'replay')
LEADAT = os.path.join(HERE, 'leadat.py')

VARIANTS = [
	'shadow=3 velw=4 latq=0.01 velsym=1 ge=0 hnow=0 beam=100000',
	'shadow=2 velw=4 latq=0.01 velsym=1 ge=0 hnow=0',
	'shadow=3 velw=10 latq=0.02 velsym=1 ge=0 hnow=0',
	'shadow=2 velw=4 latq=0.01 ge=0.04 hnow=300',
	'shadow=2 velw=1 latq=0.003 ge=0.02 hnow=600',
	'shadow=2 velw=2 latq=0.005 velsym=1 ge=0.01 hnow=150',
]


def leads(best, files):
	out = subprocess.run(['python3', LEADAT, best] + files, capture_output=True, text=True).stdout
	r = []
	for l in out.splitlines():
		m = re.match(r'(\S+)\s+rt (\d+) lead\s+([-+\d.]+) dist\s+(\d+) \|v\|\s+([\d.]+) \(ref\s+([\d.]+)\)', l)
		if m:
			r.append((float(m[3]), m[1], int(m[4]), float(m[5]), float(m[6])))
	return r


def main():
	best, prefix, d = sys.argv[1:4]
	o = dict(a.split('=', 1) for a in sys.argv[4:])
	ends = o['ends'].split(',')
	cores = int(o.get('cores', 4))
	beam = o.get('beam', '50000')
	keep = int(o.get('keep', 2))
	back = int(o.get('back', 15))  # restart each stage this many ticks before the previous dump's end
	os.makedirs(d, exist_ok=True)
	logf = open(os.path.join(d, 'stage.log'), 'a')

	def log(m):
		print(m, flush=True)
		logf.write(m + '\n')
		logf.flush()

	kept = [prefix]
	for si, h in enumerate(ends):
		jobs = []
		n = 0
		cutp = []
		for p in kept:
			if True:
				lines = open(p).read().splitlines()
				q = os.path.join(d, f'cut{si}_{os.path.basename(p)}')
				with open(q, 'w') as f:
					f.write('\n'.join(lines[:len(lines) - back]) + '\n')
				cutp.append(q)
			else:
				cutp.append(p)
		for p in cutp:
			for v in VARIANTS[:max(1, cores // len(kept)) if len(kept) > 1 else cores]:
				n += 1
				out = os.path.join(d, f's{si}_{n}.txt')
				for f in [out, out + '.1']:
					if os.path.exists(f):
						os.remove(f)
				args = [TOOL, MAP, f'best={best}', f'prefix={p}', f'beam={beam}', 'threads=1', 'rothook=2', 'abort=40', f'out={out}'] + v.split()
				if h != 'end':
					args += [f'dumpat={h}', 'dumpk=2']
				jobs.append((subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True), out, p, v))
		files = []
		for pr, out, p, v in jobs:
			txt = pr.communicate()[0]
			m = re.search(r'RESULT (.*)', txt)
			log(f'stage {si} ({h}) from {os.path.basename(p)} [{v}] -> {m.group(1) if m else "crash"}')
			for f in [out, out + '.1']:
				if os.path.exists(f):
					files.append(f)
		if h == 'end':
			for f in files:
				log(f'complete: {f}')
				subprocess.run(['cp', f, os.path.join(d, 'best_' + os.path.basename(f))])
			break
		r = sorted(leads(best, files), reverse=True)
		for ld, f, dist, v, rv in r:
			log(f'  {os.path.basename(f)} lead {ld:+.2f} dist {dist} |v| {v} (ref {rv})')
		if not r:
			log('no dumps')
			return
		kept = [f for _, f, _, _, _ in r[:keep]]
	log('done')


if __name__ == '__main__':
	main()
