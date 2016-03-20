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

import collections
import json
import os
import re

CompileCommand = collections.namedtuple('CompileCommand', ['src_path', 'invokation', 'work_dir'])

def read_compile_commands(source_dir, build_dir):
    src_path_exclude_regex = re.compile(r"external/.*")
    commands = {}

    with open(os.path.join(build_dir, 'compile_commands.json')) as file:
        for command in json.load(file):
            src_path = os.path.relpath(os.path.join(command['directory'], command['file']),
                                       start=source_dir)

            if src_path_exclude_regex.match(src_path) is None:
                commands[src_path] = CompileCommand(src_path,
                                                    command['command'],
                                                    command['directory'])

    return commands
