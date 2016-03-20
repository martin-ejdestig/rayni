#!/usr/bin/env python
#
# This file is part of Rayni.
#
# Copyright (C) 2016 Martin Ejdestig <marejde@gmail.com>
#
# Rayni is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Rayni is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Rayni. If not, see <http://www.gnu.org/licenses/>.

# pylint: disable=missing-docstring,fixme

# TODO:
# This script would not be needed if Clang's scan-build was better. (Need
# to filter out source in external/, warnings not understood by Clang are not
# filtered out, etc.). An improved Python implementation was checked in to
# Clang's repository early 2016. Uncertain to what extent it will be shipped
# by Linux distributions when they package Clang 3.8 since it has not yet
# replaced the Perl based implementation upstream.
#
# Investigate using the Python based version when it is more widely distributed
# and remove/rework this script (still want a simple analyze build target so may
# still need to wrap scan-build-py in something).
#
# Should also look into making it easy to analyze without having a pre-existing
# build directory (optionally put one in /tmp?). Looks like scan-build-py will
# make this a bit easier. (Can generate a compilation database by itself.) Still
# need Meson to generate output from .in files etc. though, so... hummm.
#
# Should also look into making it easy to run the analyzer on parts of the
# source tree like in the Make based build system of the main repository. Not
# fun to run through all the source over and over again when working on fixing
# a single issue.

import concurrent.futures
import os
import re
import subprocess
import sys

from compile_commands import read_compile_commands
from progress_printer import ProgressPrinter

def analyzer_invocation_for_command(command):
    args = command.invokation
    args = re.sub(r"^.*?\+\+", 'clang++ --analyze -Xanalyzer -analyzer-output=text', args)
    args = re.sub(r" -c", '', args)
    args = re.sub(r" -o '?.*\.o'?", '', args)
    args = re.sub(r" '?-pipe'?", '', args)
    args = re.sub(r" '?-W[a-z0-9-=]+'?", '', args)
    args = re.sub(r" '?-M(?:[MGPD]|MD)?'?(?= )", '', args)
    args = re.sub(r" '?-M[FTQ]'? '?.*?\.[do]'?(?= )", '', args)
    return args

def run_analyzer(command):
    process = subprocess.Popen(analyzer_invocation_for_command(command),
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT,
                               shell=True,
                               cwd=command.work_dir,
                               universal_newlines=True)
    return process.communicate()[0]

def analyze(source_dir, build_dir):
    commands = read_compile_commands(source_dir, build_dir)
    progress_printer = ProgressPrinter()
    progress_printer.start('Analyzing source', len(commands))

    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 1) as executor:
        futures = [executor.submit(run_analyzer, c) for c in commands.values()]
        for future in concurrent.futures.as_completed(futures):
            progress_printer.result(future.result())

def main():
    if len(sys.argv) != 3:
        sys.exit('Usage: {0} <source directory> <build directory>'.
                 format(os.path.basename(sys.argv[0])))

    analyze(sys.argv[1], sys.argv[2])

if __name__ == '__main__':
    main()
