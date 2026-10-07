#!/usr/bin/env python3
"""Splice search for the pre-grenade run (earliest grenade pickup; what happens after the pickup does not matter).

usage: splice.py BEST DIR [hours=H] [cores=4]

Repeats:
  stage A: cut the best run at race tick c, beam-search up to race tick h (before a hard section) and dump the
           best-ranked state (ddsearch dumpat=h); skip if it is not estimated ahead of the best run
  stage B: from that prefix, several ddsearch variants to the pickup (no incumbent, line-following variants)
  polish:  after a new best, wide late-cut searches with the incumbent (its tail is fresh, so cheap gains are likely)
A result is kept when the plain DDNet prediction code confirms an earlier pickup. State in DIR (best.txt, log).
"""
import os
import random
import re
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.environ.get('SPLICE_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(HERE, '..', 'kog.map')
REPLAY = os.path.join(HERE, '..', 'ddnet', 'build', 'replay')


def pickup_rt(path):
	out = subprocess.run([REPLAY, MAP, path, '100000'], capture_output=True, text=True).stdout
	m = re.search(r'GRENADE at input \d+ \(tick \d+, start \d+, race tick (\d+)\)', out)
	if not m or re.search(r'frz 1', out.split('GRENADE')[0]):
		return None
	return int(m.group(1))


def run(args, timeout=7200):
	try:
		return subprocess.run([TOOL, MAP] + args, capture_output=True, text=True, timeout=timeout).stdout
	except subprocess.TimeoutExpired:
		return 'RESULT timeout'


def variant(rng):
	v = {
		'hnow': rng.choice([300, 600, 600, 1000]),
		'ge': rng.choice([0, 0.01, 0.02, 0.02, 0.04]),
		'angles': rng.choice([64, 64, 128]),
		'rothook': rng.choice([1, 2, 2]),
		'survive': rng.choice([6, 8, 8, 12]),
		'cellpos': rng.choice([4, 8, 8, 12]),
		'cellvel': rng.choice([0.5, 1, 1, 2]),
		'seed': rng.randrange(1, 1 << 30),
	}
	if rng.random() < 0.5:
		v['lp'] = rng.choice([0.03, 0.05, 0.1, 0.2])
		v['latdz'] = rng.choice([16, 24, 32])
	if rng.random() < 0.3:
		v['jitter'] = rng.choice([0.3, 0.7])
	return v


def kv(v):
	return [f'{k}={x}' for k, x in v.items()]


def main():
	best_in, d = sys.argv[1], sys.argv[2]
	opts = dict(a.split('=', 1) for a in sys.argv[3:])
	hours = float(opts.get('hours', 4))
	cores = int(opts.get('cores', 4))
	os.makedirs(d, exist_ok=True)
	best = os.path.join(d, 'best.txt')
	if not os.path.exists(best):
		shutil.copy(best_in, best)
	best_rt = pickup_rt(best)
	logf = open(os.path.join(d, 'splice.log'), 'a')

	def log(msg):
		print(msg, flush=True)
		logf.write(msg + '\n')
		logf.flush()

	def accept(path, how):
		nonlocal best_rt
		rt = pickup_rt(path)
		if rt is not None and rt < best_rt:
			best_rt = rt
			shutil.copy(path, best)
			shutil.copy(path, os.path.join(d, f'best_{rt}.txt'))
			log(f'*** NEW BEST {rt} ({how})')
			return True
		return False

	def parallel(jobs):
		# jobs: list of (args, out); run them at once (each single threaded)
		procs = []
		for args, out in jobs:
			if os.path.exists(out):
				os.remove(out)
			procs.append((subprocess.Popen([TOOL, MAP] + args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True), out, args))
		res = []
		for p, out, args in procs:
			txt = p.communicate()[0]
			m = re.search(r'RESULT (.*)', txt)
			res.append((m.group(1) if m else 'crash', out, args))
		return res

	def polish(n):
		# wide late cuts with the incumbent on the current best
		rng = random.Random()
		for k in range(n):
			jobs = []
			for w in range(cores // 2):
				cut = rng.randint(max(0, best_rt - 230), best_rt - 5)
				v = variant(rng)
				v['beam'] = rng.choice([50000, 100000])
				out = os.path.join(d, f'pol_{w}.txt')
				jobs.append((['best=' + best, f'cut={cut}', 'threads=2', 'out=' + out] + kv(v), out))
			for r, out, args in parallel(jobs):
				log(f'polish {" ".join(a for a in args if not a.startswith("out=") and not a.startswith("best="))} -> {r}')
				if os.path.exists(out):
					accept(out, f'polish cut {args[1]}')

	rng = random.Random(int(time.time()))
	end = time.time() + hours * 3600
	log(f'start best {best_rt}')
	polish(int(opts.get('polish0', 2)))
	it = 0
	while time.time() < end:
		it += 1
		cut = rng.randint(-40, best_rt - 120)
		horizon = rng.randint(cut + 100, min(cut + 450, best_rt - 25))
		va = variant(rng)
		va['beam'] = rng.choice([30000, 50000, 80000])
		dump = os.path.join(d, 'dump.txt')
		if os.path.exists(dump):
			os.remove(dump)
		t0 = time.time()
		txt = run(['best=' + best, f'cut={cut}', f'dumpat={horizon}', f'threads={cores}', 'out=' + dump] + kv(va))
		m = re.search(r'RESULT dump est ([\d.]+) at rt (\d+)', txt)
		if not m:
			r = re.search(r'RESULT (.*)', txt)
			log(f'it {it} A cut {cut} h {horizon} -> {r.group(1) if r else "crash"} [{time.time() - t0:.0f}s]')
			continue
		est = float(m.group(1))
		log(f'it {it} A cut {cut} h {horizon} beam {va["beam"]} -> dump est {est:.2f} [{time.time() - t0:.0f}s]')
		if est > best_rt - 1.0:
			continue
		# stage B: several variants from the dump, one core each
		jobs = []
		for w in range(cores):
			vb = variant(rng)
			vb['beam'] = rng.choice([30000, 50000, 50000])
			out = os.path.join(d, f'b_{w}.txt')
			jobs.append((['best=' + best, 'prefix=' + dump, 'threads=1', 'slack=0', 'abort=40', 'out=' + out] + kv(vb), out))
		t0 = time.time()
		got = False
		for r, out, args in parallel(jobs):
			log(f'it {it} B {" ".join(a for a in args if a.startswith(("lp", "hnow", "ge", "rothook", "beam")))} -> {r}')
			if os.path.exists(out) and accept(out, f'splice it {it} cut {cut} h {horizon}'):
				got = True
		log(f'it {it} B done [{time.time() - t0:.0f}s]')
		if got:
			polish(3)
	log(f'end best {best_rt}')


if __name__ == '__main__':
	main()
