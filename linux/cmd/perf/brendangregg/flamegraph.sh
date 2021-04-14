#!/bin/bash
# 荣涛 2021年4月14日

FLAMEGRAPH_ROOT=/work/workspace/FlameGraph-1.0/

function record() {
	pid=$1
	sec=$2
	echo "Ctrl-C to terminate recor, then generate svg file..."
	perf record -F 99 -p $pid -g -- sleep $sec
}

function gen_svg() {
	perf script > out.perf
	$FLAMEGRAPH_ROOT/stackcollapse-perf.pl out.perf > out.stack
	$FLAMEGRAPH_ROOT/flamegraph.pl out.stack > out.svg
	#mv out.svg /work/workspace/out.svg
	rm -f out.perf perf.data* out.stack
	echo "Save to out.svg"
}

if [ $# -lt 2 ]; then
	echo $0 [PID] [Second]
	exit 1
fi

record $1 $2
gen_svg
