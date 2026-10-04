#!/bin/bash
# a full run from the spawn, recorded: fulld.sh <tag> [seconds] [seed]
cd /home/user/ddnet/build-sim
T=$1; S=${2:-2400}; D=${3:-2000}
SIM_DEMO_DIR=/home/user/runs/demos HH_DEMO=stronghold_$T HH_DET=$D TAS_MAP=maps/Stronghold.map HH_ROUTE=../data/hookbot/routes/Stronghold.txt HH_X0=33 HH_X1=30 HH_Y=60 HH_SECONDS=$S HH_EVERY=25 HH_DEBUGTEAM=1 timeout 40000 ./testrunner --gtest_filter=SimMapBots.Hammerhit > /home/user/runs/full_$T.log 2>&1
echo "full $T: $(grep -E '^hammerhit|^route: reached|FINISH both|^wrote' /home/user/runs/full_$T.log | tr '\n' ' ')"
