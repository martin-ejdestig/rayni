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

import concurrent.futures
import difflib
import glob
import itertools
import os
import re
import subprocess
import sys

from compile_commands import read_compile_commands
from progress_printer import ProgressPrinter

CLANG_TIDY_NOISE_LINES = [
    r"[0-9]+ warnings? (and [0-9]+ errors? )?generated.",
    r"Suppressed [0-9]+ warnings? \([0-9]+ in non-user code(, [0-9]+ NOLINT)?\).",
    r"Use -header-filter=.* to display errors from all non-system headers."
]

CLANG_TIDY_NOISE_REGEX = re.compile('(?m)^(' + '|'.join(CLANG_TIDY_NOISE_LINES) + ')$')

def run_clang_format(path, content):
    with subprocess.Popen('clang-format',
                          stdin=subprocess.PIPE,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          universal_newlines=True) as process:
        stdout, stderr = process.communicate(input=content)
        if process.returncode != 0:
            return stderr

    output = []
    formatted_lines = stdout.splitlines(True)
    matcher = difflib.SequenceMatcher(None, content.splitlines(True), formatted_lines)

    for group in matcher.get_grouped_opcodes(0):
        for tag, i1, _, j1, j2 in group: # Names match docs. pylint: disable=invalid-name
            if tag in {'replace', 'insert'}:
                output.append('{}:{}: error: wrong format, change to:\n'.format(path, i1 + 1))
                output.extend(formatted_lines[j1:j2])
            elif tag == 'delete':
                # TODO: Correct? Or can delete mean something else? Meh... +/- diff instead?
                output.append('{}:{}: error: remove white space\n'.format(path, i1 + 1))

    return ''.join(output).strip()

def run_clang_tidy(compile_command):
    args = re.sub(r"^.*?\+\+", 'clang-tidy {} -- '.format(compile_command.file),
                  compile_command.invokation)
    args = re.sub(r" '?-W[a-z0-9-=]+'?", '', args)

    with subprocess.Popen(args,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          shell=True,
                          cwd=compile_command.work_dir,
                          universal_newlines=True) as process:
        output = process.communicate()[0]

    return CLANG_TIDY_NOISE_REGEX.sub('', output).strip()

def string_index_to_line_and_column(string, index):
    line = 1
    column = 1

    for line_length in [len(line) for line in string.splitlines(True)]:
        if index < line_length:
            column = index + 1
            break
        index -= line_length
        line += 1

    return (line, column)

def path_to_include_guard(path):
    guard_path, ext = os.path.splitext(os.path.relpath(path, start='src'))
    if ext == '.in':
        guard_path, _ = os.path.splitext(guard_path)

    return 'RAYNI_' + guard_path.replace(os.path.sep, '_').upper() + '_H'

def check_header_include_guard(path, content):
    match = re.match(r"^(?:\s*|/\*.*?\*/|//[^\n]*)*"
                     r"#ifndef\s+(\S*)\s*\n\s*"
                     r"#define\s(\S*).*\n.*"
                     r"#endif\s+//\s+(\S*)\s*$",
                     content,
                     flags=re.DOTALL)
    if not match:
        return '{}: error: missing include guard'.format(path)

    guard = path_to_include_guard(path)

    error_positions = [string_index_to_line_and_column(content, match.start(group))
                       for group, found_guard in enumerate(match.groups(), start=1)
                       if found_guard != guard]

    output = ['{}:{}:{}: error: include guard name should be {}'.format(path, *position, guard)
              for position in error_positions]

    return '\n'.join(output)

def style_check_file(source_dir, path, compile_command):
    outputs = []

    with open(os.path.join(source_dir, path)) as file:
        content = file.read()

    if compile_command:
        outputs.append(run_clang_tidy(compile_command))

    if path.endswith(('.h', '.h.in')):
        outputs.append(check_header_include_guard(path, content))

    outputs.append(run_clang_format(path, content))

    return '\n'.join([o for o in outputs if o])

def find_source_paths(source_dir):
    src_dirs_to_check = ['src']
    src_suffixes = ['.cpp', '.h', '.h.in']
    paths = []

    for directory, suffix in itertools.product(src_dirs_to_check, src_suffixes):
        pattern = os.path.join(source_dir, directory, '**', '*' + suffix)
        paths += [os.path.relpath(path, start=source_dir)
                  for path in glob.glob(pattern, recursive=True)]

    return sorted(paths)

def style_check(source_dir, build_dir):
    source_paths = find_source_paths(source_dir)
    compile_commands = read_compile_commands(source_dir, build_dir)

    progress_printer = ProgressPrinter()
    progress_printer.start('Checking source', len(source_paths))

    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 1) as executor:
        futures = [executor.submit(style_check_file, source_dir, path, compile_commands.get(path))
                   for path in source_paths]
        for future in concurrent.futures.as_completed(futures):
            progress_printer.result(future.result())

def main():
    if len(sys.argv) != 3:
        sys.exit('Usage: {} <source directory> <build directory>'.
                 format(os.path.basename(sys.argv[0])))

    style_check(sys.argv[1], sys.argv[2])

if __name__ == '__main__':
    main()
