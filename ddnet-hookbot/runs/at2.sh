#!/bin/bash
# a run from a spot, the tees at different heights: at2.sh <tag> <route index> <x0> <y0> <x1> <y1> [seconds] [seed]
cd /home/user/ddnet/build-sim
T=$1; R=$2; X0=$3; Y0=$4; X1=$5; Y1=$6; S=${7:-40}; D=${8:-2000}
HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=$R HH_X0=$X0 HH_Y=$Y0 HH_X1=$X1 HH_Y1=$Y1 HH_SECONDS=$S HH_EVERY=25 HH_DEBUGTEAM=1 timeout 3600 ./testrunner --gtest_filter=SimMapBots.Hammerhit > /home/user/runs/at_$T.log 2>&1
M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' /home/user/runs/at_$T.log)
echo "at $T: max wp $M"
