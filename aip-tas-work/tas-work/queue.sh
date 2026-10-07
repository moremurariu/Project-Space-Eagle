#!/bin/bash
# queue.sh FILE: run each line (a command) of FILE, QP (default 4) at a time
cd "$(dirname "$0")"
xargs -P ${QP:-4} -I{} bash -c '{}' < "$1"
echo QDONE >> runs/queue.done
