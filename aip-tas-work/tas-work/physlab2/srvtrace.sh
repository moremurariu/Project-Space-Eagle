#!/bin/bash
# srvtrace.sh INPUTS OUT: replay INPUTS on the real server code (testrunner TasReplay.Run, TAS_TRACE=1) and keep the trace
# (lines "input x y vx vy frz=F"); prints the start tick and the last 3 trace lines.
HERE=$(cd "$(dirname "$0")"; pwd)
IN=$(readlink -f "$1")
OUT=$(readlink -f "$2" 2>/dev/null || echo "$2")
(cd $HERE/../../ddnet/build-sim && TAS_MAP=$HERE/../AiP-Gores.map TAS_INPUTS=$IN TAS_TRACE=1 ./testrunner --gtest_filter=TasReplay.Run > "$OUT" 2>&1)
grep -o "start tick -\?[0-9]*" "$OUT" | head -1
grep -E "^[0-9]+ [0-9.-]+ [0-9.-]+ [0-9.-]+ [0-9.-]+ frz=" "$OUT" | tail -3
grep -c "frz=1" "$OUT" | sed 's/^/frozen lines: /'
grep -E "DOUBLE START|tee died" "$OUT" | head -2
