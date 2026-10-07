#!/bin/bash
# srvfin.sh INPUTS: replay on the real server code (TasReplay) and print pickup / frozen / finish lines
IN=$(readlink -f "$1")
MAPF=$(readlink -f "$(dirname "$0")/AiP-Gores.map")
cd "$(dirname "$0")/../ddnet/build-sim" && TAS_MAP=$MAPF TAS_INPUTS=$IN ./testrunner --gtest_filter=TasReplay.Run 2>&1 | grep -E 'pickup|frozen|finish tick|died|DOUBLE|OK|FAILED'
