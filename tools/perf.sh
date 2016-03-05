#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $(basename $0) <command> [<args>...]"
	exit 1
fi

PERF_DATA=${MESON_BUILD_ROOT:-.}/perf.data

perf record -o $PERF_DATA --call-graph dwarf $@
perf report -i $PERF_DATA
