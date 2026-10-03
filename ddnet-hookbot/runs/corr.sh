#!/bin/bash
# after the shaft: from the corridor at x 130 (route waypoint 144), one seed per argument, 40 s each
cd /home/user/ddnet/build-sim
for D in "$@"; do
  HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=144 HH_X0=130 HH_Y=174 HH_X1=132 HH_SECONDS=${SECS:-40} HH_EVERY=25 HH_DEBUGTEAM=1 timeout 3600 ./testrunner --gtest_filter=SimMapBots.Hammerhit > /home/user/runs/corr_$D.log 2>&1
  M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' /home/user/runs/corr_$D.log)
  echo "corr $D: max wp $M"
done
