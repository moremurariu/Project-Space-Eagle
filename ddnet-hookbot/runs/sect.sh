#!/bin/bash
# a section from one spot with several seeds, one at a time: sect.sh <tag> <route index> <x0> <x1> <y> <seconds> <until wp> <seed>...
cd /home/user/ddnet/build-sim
T=$1; R=$2; X0=$3; X1=$4; Y=$5; S=$6; U=$7; shift 7
for D in "$@"; do
  L=/home/user/runs/sect_${T}_$D.log
  HH_SOLODBG=${SOLODBG:-} HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_ROUTE_AT=$R HH_ROUTE_UNTIL=$U HH_X0=$X0 HH_Y=$Y HH_X1=$X1 HH_SECONDS=$S HH_EVERY=25 HH_DEBUGTEAM=1 timeout 3600 ./testrunner --gtest_filter=SimMapBots.Hammerhit > $L 2>&1
  M=$(awk '/^route/ && !/\[1\]/ { if ($0 ~ /restarting/) exit; if (match($0, /next: [0-9]+/)) { n = substr($0, RSTART+6, RLENGTH-6)+0; if (n > m) m = n } } END { print m+0 }' $L)
  echo "$T seed $D: max wp $M $(grep -q restarting $L && echo RESTART) $(grep -E '^both past' $L)"
done
