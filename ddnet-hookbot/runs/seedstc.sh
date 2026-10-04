#!/bin/bash
# full runs from the spawn at several timing seeds, one at a time: seeds.sh <tag> <seed>... ; prints each result
cd /home/user/ddnet/build-sim
T=$1; shift
for D in "$@"; do
  HH_TEAMCALLS=1 HH_PROF=1 HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_X0=33 HH_X1=30 HH_Y=60 HH_SECONDS=900 HH_EVERY=50 HH_DEBUGTEAM=1 timeout 3000 ./testrunner --gtest_filter=SimMapBots.Hammerhit > /home/user/runs/seed_${T}_$D.log 2>&1
  R=$(grep -c "restarting" /home/user/runs/seed_${T}_$D.log)
  F=$(grep -E "^FINISH both" /home/user/runs/seed_${T}_$D.log)
  C=$(grep -E "^prof Tick" /home/user/runs/seed_${T}_$D.log | awk '{print $3}')
  W=$(grep -oE "^route: reached waypoint [0-9]+" /home/user/runs/seed_${T}_$D.log | awk '{print $4}')
  echo "$T seed $D: ${F:-no finish}, restarts $R, max wp $W, brain ${C}s"
done
