#!/bin/bash
# per attempt in a seed log: the furthest waypoint and how it ended. attempts.sh <log>
awk '/^route [0-9.]+ s: next: [0-9]+\// { split($5,a,"/"); w=a[1]+0; if(w>m) m=w }
     /^route [0-9.]+ s: [^[]*restarting/ { t=$2; sub(/^route [0-9.]+ s: /,""); printf "  attempt to %s s: max wp %d (%s)\n", t, m, $0; m=0 }
     /^FINISH both/ { printf "  FINISH: %s\n", $0 }
     END { printf "  last attempt: max wp %d\n", m }' "$1"
