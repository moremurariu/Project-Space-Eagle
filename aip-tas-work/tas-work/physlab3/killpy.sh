#!/bin/bash
# killpy.sh PATTERN: kill python3 processes (only python3, never shells) whose command line contains PATTERN
for p in $(pgrep -x python3); do
  if tr '\0' ' ' < /proc/$p/cmdline 2>/dev/null | grep -q -- "$1"; then kill $p; echo killed $p; fi
done
