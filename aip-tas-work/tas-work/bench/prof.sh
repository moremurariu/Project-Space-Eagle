#!/bin/bash
# prof.sh N CMD...: poor man's profiler. Runs CMD, samples its main thread's stack N times with gdb, prints the
# most frequent innermost frames and the most frequent frames anywhere on the stack (inclusive).
N=$1; shift
"$@" > /dev/null 2>&1 &
PID=$!
sleep 3
OUT=$(mktemp)
for i in $(seq 1 $N); do
	kill -0 $PID 2> /dev/null || break
	gdb -p $PID -batch -ex "thread apply all bt 25" 2> /dev/null | grep -E "^#" >> $OUT
	echo "----" >> $OUT
	sleep 0.3
done
kill $PID 2> /dev/null
echo "== innermost frames (self)"
awk '/^#0 /' $OUT | sed -E 's/^#0 +(0x[0-9a-f]+ in )?//; s/ \(.*//' | sort | uniq -c | sort -rn | head -15
echo "== inclusive"
sed -E 's/^#[0-9]+ +(0x[0-9a-f]+ in )?//; s/ \(.*//' $OUT | grep -v "^----" | sort | uniq -c | sort -rn | head -30
rm -f $OUT
