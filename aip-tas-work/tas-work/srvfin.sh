#!/bin/bash
# srvfin.sh INPUTS: replay on the real server code (TasReplay) built several ways and print pickup / frozen / finish.
# A run is valid only if every build finishes it with the same time and no frozen tick: float rounding differs between
# platforms (fused multiply-add on ARM64 or x86 -march=native, other compilers), and a 1/256 px/t velocity difference
# can grow into a 1 px position difference that decides a freeze-tile corner (Oct 11: 2534 / 2533 / 2531 finished on
# the plain x86-64 build but froze at line 2167 on the user's server and on a -mfma build).
# Builds: ../ddnet/build-sim (gcc, plain x86-64), ../ddnet-up/build-fma (gcc -mfma -ffp-contract=fast),
# ../ddnet-up/build-clang (clang -mfma -ffp-contract=on), each if present.
# On success the standard lines are printed once ("frozen ticks 0", "start tick S finish tick F -> N ticks"); on a
# mismatch only "MISMATCH" lines, so callers that look for "-> N ticks" and "frozen ticks 0" reject the run.
IN=$(readlink -f "$1")
HERE=$(cd "$(dirname "$0")" && pwd)
MAPF=$HERE/AiP-Gores.map
OUT=""
REF=""
OK=1
for B in "$HERE/../ddnet/build-sim" "$HERE/../ddnet-up/build-fma" "$HERE/../ddnet-up/build-clang"; do
	[ -x "$B/testrunner" ] || continue
	R=$(cd "$B" && TAS_MAP=$MAPF TAS_INPUTS=$IN ./testrunner --gtest_filter=TasReplay.Run 2>&1 | grep -E 'pickup|frozen|finish tick|died|DOUBLE|OK|FAILED')
	NAME=$(basename "$B")
	FIN=$(echo "$R" | grep -o -- '-> -\?[0-9]* ticks')
	FRZ=$(echo "$R" | grep -o 'frozen ticks [0-9]*')
	if [ -z "$REF" ]; then
		REF="$FIN|$FRZ"
		OUT="$R"
	fi
	echo "$FRZ" | grep -q 'frozen ticks 0$' || OK=0
	echo "$FIN" | grep -q -- '-> [0-9][0-9]* ticks' || OK=0
	[ "$FIN|$FRZ" = "$REF" ] || OK=0
	SUM="$SUM
$NAME: $FIN, $FRZ"
done
if [ $OK = 1 ]; then
	echo "$OUT"
	echo "builds agree:$SUM"
else
	echo "MISMATCH between builds (or no finish / frozen):$SUM" | sed 's/-> \([0-9-]*\) ticks/=> \1 race ticks/; s/frozen ticks/frozen/'
fi
