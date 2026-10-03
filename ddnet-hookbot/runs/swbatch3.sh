#!/bin/bash
# swing course section: start positions varied (the timing seed alone hardly varies the runs any more), 25 s each, one at
# a time; prints the furthest waypoint before the first restart. Args: tag, then x0:x1 pairs
cd /home/user/ddnet/build-sim
TAG=$1; shift
for P in "$@"; do
  X0=${P%%:*}; X1=${P##*:}
  HH_DET=2000 TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=122 HH_X0=$X0 HH_Y=225 HH_X1=$X1 HH_SECONDS=35 HH_EVERY=25 HH_DEBUGTEAM=1 timeout 3600 ./testrunner --gtest_filter=SimMapBots.Hammerhit > /home/user/runs/sw${TAG}_$X0-$X1.log 2>&1
  M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' /home/user/runs/sw${TAG}_$X0-$X1.log)
  echo "$TAG $X0:$X1: max wp $M"
done
