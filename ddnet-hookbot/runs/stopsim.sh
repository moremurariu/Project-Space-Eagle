#!/bin/bash
# stops the run scripts and the sim (by command lines that start with them, so the calling shell isn't hit)
for p in $(pgrep -f "^/bin/bash \./(sec|seeds|at|sw)\.sh"); do kill $p; done
for p in $(pgrep -x testrunner); do kill $p; done
sleep 1
pgrep -x testrunner && echo "still running" || echo "stopped"
