#!/usr/bin/env python3
# simcmp.py INPUTS: replay INPUTS in the simulator (lab) and on the real server (testrunner TasReplay.Run) and
# compare positions/velocities on every tick; prints the server's pickup / start / freeze summary.
import os, re, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
inp = os.path.abspath(sys.argv[1])
lab = subprocess.run([os.path.join(here, '../ddnet/build-sim/lab'), os.path.join(here, 'AiP-Gores.map'), 'replay ' + inp],
                     capture_output=True, text=True).stdout
sim = {}
for l in lab.splitlines():
    m = re.search(r't=(\d+) pos ([\d.-]+) ([\d.-]+) .*vel ([\d.-]+) ([\d.-]+)', l)
    if m:
        sim[int(m.group(1))] = tuple(float(m.group(i)) for i in range(2, 6))
env = dict(os.environ, TAS_MAP=os.path.join(here, 'AiP-Gores.map'), TAS_INPUTS=inp, TAS_TRACE='1')
srv_out = subprocess.run(['./testrunner', '--gtest_filter=TasReplay.Run'], cwd=os.path.join(here, '../ddnet/build-sim'),
                         env=env, capture_output=True, text=True).stdout
srv = {}
for l in srv_out.splitlines():
    m = re.match(r'(\d+) ([\d.-]+) ([\d.-]+) ([\d.-]+) ([\d.-]+) frz=(\d)', l)
    if m:
        srv[int(m.group(1))] = tuple(float(m.group(i)) for i in range(2, 6))
diff = [t for t in sorted(srv) if t in sim and max(abs(a - b) for a, b in zip(sim[t], srv[t])) > 0.002]
print('ticks sim %d server %d, first mismatch %s (%d mismatches)' % (len(sim), len(srv), diff[0] if diff else None, len(diff)))
for l in srv_out.splitlines():
    if re.match(r'(grenade pickup|frozen|start tick|DOUBLE|tee died)', l):
        print('server:', l)
