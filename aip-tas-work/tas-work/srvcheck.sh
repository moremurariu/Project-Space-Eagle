#!/bin/bash
# srvcheck.sh INPUTS [STOPX]: replay a run on the real server (testrunner, TasReplay.Run with TAS_TRACE=1) and report
# the start tick, the race tick when x first exceeds STOPX (default 8800), frozen ticks and double starts.
# Needs ../ddnet/build-sim/testrunner (cd ../ddnet/build-sim && ninja testrunner).
cd "$(dirname "$0")"
IN=$(readlink -f "$1")
STOPX=${2:-8800}
LOG=$(mktemp)
(cd ../ddnet/build-sim && TAS_MAP=$(readlink -f ../../tas-work/AiP-Gores.map) TAS_INPUTS=$IN TAS_TRACE=1 \
	./testrunner --gtest_filter=TasReplay.Run > "$LOG" 2>&1)
python3 - "$LOG" "$STOPX" <<'PY'
import re, sys
txt = open(sys.argv[1]).read()
stopx = float(sys.argv[2])
pos = {}
for l in txt.splitlines():
    m = re.match(r'(\d+) ([\d.-]+) ([\d.-]+) ([\d.-]+) ([\d.-]+) frz=(\d)', l)
    if m:
        pos[int(m.group(1))] = (float(m.group(2)), int(m.group(6)))
m = re.search(r'start tick (-?\d+)', txt)
start = int(m.group(1)) if m else None
gate = next((n for n in sorted(pos) if pos[n][0] > stopx), None)
print('server: start tick %s, x > %g at input %s -> race ticks %s, frozen ticks %d, double start %s, died %s' % (
    start, stopx, gate, gate - start if gate is not None and start is not None else None,
    sum(f for x, f in pos.values()), 'DOUBLE START' in txt, 'tee died' in txt))
PY
rm -f "$LOG"
