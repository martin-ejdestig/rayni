#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $(basename $0) <command> [<args>...]"
	exit 1
fi

perf record --call-graph dwarf $@
perf report
