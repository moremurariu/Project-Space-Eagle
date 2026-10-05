#!/usr/bin/env python3
"""Large neighbourhood search on the pre-grenade run with ddsearch (exact fast stepper).

usage: lns.py BEST DIR [hours=H] [workers=N] [cutmin=RT] [cutmax=RT]

Repeats: cut the current best at a random race tick, re-search to the grenade pickup with a random ddsearch
variant (the best run's continuation stays in the beam), keep verified earlier pickups. State in DIR:
best.txt (current best inputs), best_<rt>.txt, lns.log.
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
ROT = [int(x) for x in os.environ.get('LNS_ROT', '0,1,1').split(',')]
TOOL = os.environ.get('LNS_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(HERE, '..', 'kog.map')
REPLAY = os.path.join(HERE, '..', 'ddnet', 'build', 'replay')


def pickup_rt(path):
	out = subprocess.run([REPLAY, MAP, path, '100000'], capture_output=True, text=True).stdout
	m = re.search(r'GRENADE at input \d+ \(tick \d+, start \d+, race tick (\d+)\)', out)
	if not m or 'frz 1' in out:
		return None
	return int(m.group(1))


def variant(rng):
	v = {
		'beam': rng.choice([4000, 8000, 8000, 16000, 30000]),
		'hnow': rng.choice([300, 600, 600, 1000]),
		'ge': rng.choice([0, 0.01, 0.02, 0.02, 0.04]),
		'angles': rng.choice([64, 64, 128]),
		'rothook': rng.choice(ROT),
		'survive': rng.choice([6, 8, 8, 12]),
		'cellpos': rng.choice([4, 8, 8, 12]),
		'cellvel': rng.choice([0.5, 1, 1, 2]),
		'seed': rng.randrange(1, 1 << 30),
	}
	if rng.random() < 0.4:
		v['lp'] = rng.choice([0.03, 0.05, 0.1])
		v['latdz'] = rng.choice([16, 24, 32])
	if rng.random() < 0.4:
		v['jitter'] = rng.choice([0.3, 0.7, 1.5])
	return v


def main():
	best_in = sys.argv[1]
	d = sys.argv[2]
	kv = dict(a.split('=', 1) for a in sys.argv[3:])
	hours = float(kv.get('hours', 1))
	workers = int(kv.get('workers', 4))
	cutmin = int(kv.get('cutmin', 5))
	cutmax = int(kv.get('cutmax', 10000))
	latep = float(kv.get('latep', 0))
	latemin = int(kv.get('latemin', 780))
	os.makedirs(d, exist_ok=True)
	best = os.path.join(d, 'best.txt')
	if not os.path.exists(best):
		shutil.copy(best_in, best)
	best_rt = pickup_rt(best)
	log = open(os.path.join(d, 'lns.log'), 'a')
	lock = threading.Lock()
	state = {'rt': best_rt, 'job': 0}
	end = time.time() + hours * 3600
	print(f'start: best {best_rt}', flush=True)
	log.write(f'start best {best_rt}\n')
	log.flush()

	def worker(wid):
		rng = random.Random(wid * 1000003 + int(time.time()))
		while time.time() < end:
			with lock:
				state['job'] += 1
				job = state['job']
				cur_rt = state['rt']
				src = os.path.join(d, f'src_{wid}.txt')
				shutil.copy(best, src)
			if rng.random() < latep:
				cut = rng.randint(max(cutmin, latemin), min(cutmax, cur_rt - 3))
			else:
				cut = rng.randint(cutmin, min(cutmax, cur_rt - 3))
			v = variant(rng)
			out = os.path.join(d, f'out_{wid}.txt')
			if os.path.exists(out):
				os.remove(out)
			args = [TOOL, MAP, f'best={src}', f'cut={cut}', 'threads=1', f'out={out}'] + [f'{k}={x}' for k, x in v.items()]
			t0 = time.time()
			try:
				res = subprocess.run(args, capture_output=True, text=True, timeout=3600).stdout
			except subprocess.TimeoutExpired:
				res = 'RESULT timeout'
			m = re.search(r'RESULT (.*)', res)
			r = m.group(1) if m else 'crash'
			line = f'job {job} w{wid} cut {cut} {" ".join(f"{k}={x}" for k, x in v.items())} -> {r} [{time.time() - t0:.0f}s]'
			with lock:
				log.write(line + '\n')
				log.flush()
				if os.path.exists(out):
					rt = pickup_rt(out)
					if rt is not None and rt < state['rt']:
						state['rt'] = rt
						shutil.copy(out, best)
						shutil.copy(out, os.path.join(d, f'best_{rt}.txt'))
						msg = f'*** NEW BEST {rt} (job {job}, cut {cut})'
						print(msg, flush=True)
						log.write(msg + '\n')
						log.flush()

	ths = [threading.Thread(target=worker, args=(w,)) for w in range(workers)]
	for t in ths:
		t.start()
	for t in ths:
		t.join()
	print(f'end: best {state["rt"]}', flush=True)


if __name__ == '__main__':
	main()
