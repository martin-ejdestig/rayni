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

# pylint: disable=missing-docstring

import os
import re
import subprocess
import sys

from compile_commands import read_compile_commands

def print_assembler_invokation_for_command(command):
    args = command.invokation
    args = re.sub(r" -c ", ' -S ', args)
    args = re.sub(r" -o '?.*\.o'? ", ' -o- ', args)
    args = re.sub(r" '?-M(?:[MGPD]|MD)?'?(?= )", '', args)
    args = re.sub(r" '?-M[FTQ]'? '?.*?\.[do]'?(?= )", '', args)
    return args

def print_assembler(source_dir, build_dir, source_file):
    src_relpath = os.path.relpath(source_file, start=source_dir)
    command = read_compile_commands(source_dir, build_dir).get(src_relpath)
    if not command:
        sys.exit('Do not know how to compile "{0}".'.format(source_file))

    with subprocess.Popen(print_assembler_invokation_for_command(command),
                          cwd=command.work_dir,
                          shell=True) as process:
        if process.wait() != 0:
            sys.exit('Failed to run compiler command for outputting assembler.')

def main():
    if len(sys.argv) != 4:
        sys.exit('Usage: {0} <source directory> <build directory> <source file>'.
                 format(os.path.basename(sys.argv[0])))

    print_assembler(sys.argv[1], sys.argv[2], sys.argv[3])

if __name__ == '__main__':
    main()
