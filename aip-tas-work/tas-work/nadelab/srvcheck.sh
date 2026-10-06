#!/bin/bash
# srvcheck.sh MAP INPUTS [EXTRA=200]: replay INPUTS on the real DDNet server code (testrunner TasReplay.Run, TAS_TRACE=1)
# with EXTRA idle ticks appended, and report the height reached (standing y 12785 minus min y; also vs the spawn y).
# Needs ../../ddnet/build-sim/testrunner (cd ddnet/build-sim && ninja testrunner).
HERE=$(cd "$(dirname "$0")" && pwd)
MAP=$(readlink -f "$1")
IN=$(readlink -f "$2")
EXTRA=${3:-200}
TMP=$(mktemp)
LOG=$(mktemp)
cp "$IN" "$TMP"
LAST=$(tail -1 "$IN" | awk '{print "0 0 0 0 "$5" "$6" "$7}')
for i in $(seq "$EXTRA"); do echo "$LAST" >> "$TMP"; done
(cd "$HERE/../../ddnet/build-sim" && TAS_MAP="$MAP" TAS_INPUTS="$TMP" TAS_TRACE=1 ./testrunner --gtest_filter=TasReplay.Run > "$LOG" 2>&1)
python3 - "$LOG" "${STAND_Y:-12785}" <<'PY'
import re, sys
txt = open(sys.argv[1]).read()
stand = float(sys.argv[2])
spawn = None
m = re.search(r'^spawn ([\d.-]+) ([\d.-]+)', txt, re.M)
if m: spawn = float(m.group(2))
pos = []
for l in txt.splitlines():
    m = re.match(r'(\d+) ([\d.-]+) ([\d.-]+) ([\d.-]+) ([\d.-]+) frz=(\d)', l)
    if m:
        pos.append((int(m.group(1)), float(m.group(2)), float(m.group(3)), float(m.group(4)), float(m.group(5))))
if not pos:
    print('server: no trace (did the test run?)'); print(txt[-2000:]); sys.exit(1)
k, x, y, vx, vy = min(pos, key=lambda p: p[2])
print('server: %d ticks traced, min y %.1f at tick %d (x %.1f) -> height %.1f px = %.2f tiles above the standing y %.0f (%.1f above the spawn y %s); died: %s' % (
    len(pos), y, k, x, stand - y, (stand - y) / 32, stand, (spawn - y) if spawn else float('nan'), spawn, 'tee died' in txt))
PY
rm -f "$TMP" "$LOG"
