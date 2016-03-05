#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Usage: $(basename $0) <command> [<args>...]"
	exit 1
fi

events=cache-references,cache-misses,branch-instructions,branch-misses,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,L1-dcache-store-misses

perf stat -e $events $@
