#!/usr/bin/env python3
"""Gate chain: carry a prefix along a reference run with short goal-directed searches.

usage: gatechain.py PREFIX REF OUTDIR [step=80] [tol=2] [beam=100000] [threads=4] [k=2] [final=1]

From the end of PREFIX, gates are placed on REF's trajectory every `step` race ticks (counted on REF's clock from the
point PREFIX has reached). Each gate is REF's position there and needs REF's speed along REF's direction (minus `tol`)
and REF's air jump if REF still has it. A geodesic gate search (ddsearch geogoal=...) finds the earliest arrival; the
arrival prefix starts the next segment. The lead over REF is printed at every gate. Finally (final=1) the last prefix
is searched to the grenade pickup against REF.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
TOOL = os.environ.get('CHAIN_TOOL', os.path.join(HERE, '..', 'ddnet', 'build', 'ddsearch'))
MAP = os.path.join(HERE, '..', 'kog.map')
REPLAY = os.path.join(HERE, '..', 'ddnet', 'build', 'replay')


def replay(path):
	out = subprocess.run([REPLAY, MAP, path, '1'], capture_output=True, text=True).stdout
	pts = {}
	start = None
	for l in out.splitlines():
		m = re.match(r't (\d+) pos ([-\d.]+) ([-\d.]+) vel ([-\d.]+) ([-\d.]+) hook (-?\d+) jumped (\d+) frz (\d) start (-?\d+)', l)
		if m:
			pts[int(m[1])] = (float(m[2]), float(m[3]), float(m[4]), float(m[5]), int(m[6]), int(m[7]), int(m[8]))
			start = int(m[9])
	g = re.search(r'race tick (\d+)\)', out)
	return pts, start, int(g.group(1)) if g else None


def main():
	prefix, ref, outdir = sys.argv[1:4]
	o = dict(a.split('=', 1) for a in sys.argv[4:])
	step = int(o.get('step', 80))
	tol = float(o.get('tol', 2))
	beam = o.get('beam', '100000')
	threads = o.get('threads', '4')
	extra = o.get('extra', '')
	os.makedirs(outdir, exist_ok=True)
	rp, rstart, rpick = replay(ref)
	cur = prefix
	seg = 0
	while True:
		pp, pstart, _ = replay(cur)
		n = max(pp)
		x, y = pp[n][0], pp[n][1]
		rt = n - pstart
		# REF tick where REF is closest to the prefix end (within 40 ticks of the same race tick)
		cand = [t for t in rp if abs((t - rstart) - rt) <= 40 and t - rstart < rpick]
		tm = min(cand, key=lambda t: (rp[t][0] - x) ** 2 + (rp[t][1] - y) ** 2)
		lead = (tm - rstart) - rt
		gate_t = tm + step
		if gate_t - rstart >= rpick - 30:
			break
		gx, gy, gvx, gvy, ghook, gj, _ = rp[gate_t]
		need_jump = 1 if not (gj & 2) else 0
		seg += 1
		out = os.path.join(outdir, f'seg{seg}.txt')
		args = [TOOL, MAP, f'best={ref}', f'prefix={cur}', f'geogoal={gx:.0f},{gy:.0f}', 'geor=40', f'geovref={gvx:.3f},{gvy:.3f}',
			f'geovtol={tol}', f'geojump={need_jump}', 'geoge=0.02', 'jw=144', 'geol=4', 'geos=30', 'angles=128', f'beam={beam}',
			f'threads={threads}', 'rothook=2', 'transplant=1', f'out={out}'] + extra.split()
		txt = subprocess.run(args, capture_output=True, text=True).stdout
		m = re.search(r'RESULT (\d+) verified (\d+) \(bad 0\)', txt)
		if not m or not os.path.exists(out):
			r = re.search(r'RESULT (.*)', txt)
			print(f'seg {seg}: from rt {rt} (lead {lead:+d}) to REF rt {gate_t - rstart} at ({gx:.0f},{gy:.0f}) -> FAILED {r.group(1) if r else "crash"}', flush=True)
			return
		arr = int(m.group(1))
		print(f'seg {seg}: from rt {rt} (lead {lead:+d}) to REF rt {gate_t - rstart} at ({gx:.0f},{gy:.0f}) jump {need_jump} -> arrival {arr}, lead {gate_t - rstart - arr:+d}', flush=True)
		cur = out
	if o.get('final', '1') == '1':
		out = os.path.join(outdir, 'final.txt')
		args = [TOOL, MAP, f'best={ref}', f'prefix={cur}', 'transplant=1', f'beam={beam}', f'threads={threads}', 'rothook=2', 'lp=0.1',
			'latdz=24', 'slack=0', 'abort=40', f'out={out}']
		txt = subprocess.run(args, capture_output=True, text=True).stdout
		r = re.search(r'RESULT (.*)', txt)
		print(f'final from rt {rt} (lead {lead:+d}): {r.group(1) if r else "crash"}', flush=True)


if __name__ == '__main__':
	main()
