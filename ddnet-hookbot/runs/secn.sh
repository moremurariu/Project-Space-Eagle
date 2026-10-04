#!/bin/bash
# a section from 22 starts at one seed (the seed hardly matters where team plans decide; start positions do):
# secn.sh <tag> <route index> <x0> <x1> <y> <through wp> <seconds> <seed>
# each start: both standing on row y, at x0+d and x1+d for d from -0.5 to +0.5 in steps of 0.1, and swapped
cd /home/user/ddnet/build-sim
TAG=$1; R=$2; X0=$3; X1=$4; Y=$5; W=$6; S=$7; D=$8
N=0; P=0
for Off in -0.5 -0.4 -0.3 -0.2 -0.1 0 0.1 0.2 0.3 0.4 0.5; do
  for Swap in 0 1; do
    A=$(echo "$X0 + $Off" | bc); B=$(echo "$X1 + $Off" | bc)
    [ $Swap = 1 ] && { T=$A; A=$B; B=$T; }
    L=/home/user/runs/secn_${TAG}_$A-$B.log
    HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=$R HH_X0=$A HH_Y=$Y HH_X1=$B HH_SECONDS=$S HH_EVERY=25 HH_DEBUGTEAM=1 timeout 1200 ./testrunner --gtest_filter=SimMapBots.Hammerhit > $L 2>&1
    M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' $L)
    N=$((N+1)); [ $M -ge $W ] && P=$((P+1))
    echo "$TAG $A:$B: max wp $M"
  done
done
echo "$TAG: $P of $N through"
