#!/bin/bash
# a section from a start, several start offsets and seeds, one run at a time:
# sec.sh <tag> <route index> <x0> <x1> <y> <through wp> <seconds> <seed>...
# each start: both standing on row y, at x0+d and x1+d for d in -0.3 0 0.3, and swapped
cd /home/user/ddnet/build-sim
TAG=$1; R=$2; X0=$3; X1=$4; Y=$5; W=$6; S=$7; shift 7
N=0; P=0
for D in "$@"; do
  for Off in -0.3 0 0.3; do
    for Swap in 0 1; do
      A=$(echo "$X0 + $Off" | bc); B=$(echo "$X1 + $Off" | bc)
      [ $Swap = 1 ] && { T=$A; A=$B; B=$T; }
      L=/home/user/runs/sec_${TAG}_${D}_$A-$B.log
      HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=$R HH_X0=$A HH_Y=$Y HH_X1=$B HH_SECONDS=$S HH_EVERY=25 HH_DEBUGTEAM=1 timeout 1200 ./testrunner --gtest_filter=SimMapBots.Hammerhit > $L 2>&1
      M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' $L)
      N=$((N+1)); [ $M -ge $W ] && P=$((P+1))
      echo "$TAG seed $D $A:$B: max wp $M"
    done
  done
done
echo "$TAG: $P of $N through"
