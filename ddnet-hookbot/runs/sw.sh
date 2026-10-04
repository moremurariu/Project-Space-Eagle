#!/bin/bash
# the swing course (from waypoint 122, both tees in the pocket at the gap's end) from 8 start positions at each seed,
# one run at a time: sw.sh <tag> <seed>... ; prints the furthest waypoint before the first restart, and a summary
# (through: waypoint 145 or more, the corridor after the shaft)
cd /home/user/ddnet/build-sim
TAG=$1; shift
N=0; P=0
for D in "$@"; do
  for Pos in 235.1:236.4 235.1:236.9 235.6:237.4 236.4:235.1 236.9:235.1 236.9:235.6 237.4:235.1 237.4:235.6; do
    X0=${Pos%%:*}; X1=${Pos##*:}
    L=/home/user/runs/sw_${TAG}_${D}_$X0-$X1.log
    HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=122 HH_X0=$X0 HH_Y=225 HH_X1=$X1 HH_SECONDS=60 HH_EVERY=25 HH_DEBUGTEAM=1 timeout 1200 ./testrunner --gtest_filter=SimMapBots.Hammerhit > $L 2>&1
    M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' $L)
    N=$((N+1)); [ $M -ge 145 ] && P=$((P+1))
    echo "$TAG seed $D $X0:$X1: max wp $M"
  done
done
echo "$TAG: $P of $N through"
