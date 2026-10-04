#!/bin/bash
# team searches by call site (line in hookbot.cpp), from a log run with HH_TEAMCALLS=1: calls, found, ticks spent on
# searches that found nothing and on ones that found something (shared calls count once). teamcalls.sh <log>
awk '/^teamcall/ && !/shared/ {
  line=$6; found=($9=="found,"); match($0, /[0-9]+ ticks/); t=substr($0, RSTART, RLENGTH-6)+0;
  calls[line]++; if(found){f[line]++; tf[line]+=t} else {tn[line]+=t}; all+=t; nall+=(found?0:t)
}
END {
  printf "%-6s %6s %6s %12s %12s\n", "line", "calls", "found", "ticks none", "ticks found";
  for(l in calls) printf "%-6s %6d %6d %12d %12d\n", l, calls[l], f[l], tn[l], tf[l] | "sort -k4 -n -r";
  close("sort -k4 -n -r");
  printf "total %d ticks, %d (%.0f%%) in searches that found nothing\n", all, nall, 100*nall/all
}' "$1"
