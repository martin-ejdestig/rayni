#!/usr/bin/env python

import json
import os
import re
import subprocess
import sys

def read_compile_commands(build_dir):
    with open(os.path.join(build_dir, 'compile_commands.json')) as file:
        return json.load(file)

def paths_point_to_same_file(path1, path2):
    return os.path.abspath(path1) == os.path.abspath(path2)

def get_compile_command(build_dir, source_file):
    for command in read_compile_commands(build_dir):
        if paths_point_to_same_file(source_file,
                                    os.path.join(command['directory'], command['file'])):
            return command
    return None

def transform_command(command):
    command = re.sub(r" -c ", ' -S ', command)
    command = re.sub(r" -o '?.*\.o'? ", ' -o- ', command)
    command = re.sub(r" '?-M(?:[MGPD]|MD)?'?(?= )", '', command)
    command = re.sub(r" '?-M[FTQ]'? '?.*?\.[do]'?(?= )", '', command)
    return command

def print_assembler(build_dir, source_file):
    command = get_compile_command(build_dir, source_file)
    if not command:
        sys.exit('Do not know how to compile "{0}".'.format(source_file))

    with subprocess.Popen(transform_command(command['command']),
                          cwd=command['directory'],
                          shell=True) as process:
        if process.wait() != 0:
            sys.exit('Failed to run compiler command for outputting assembler.')

def main():
    if len(sys.argv) != 3:
        sys.exit('Usage: {0} <build directory> <source file>'.format(os.path.basename(sys.argv[0])))

    print_assembler(sys.argv[1], sys.argv[2])

if __name__ == '__main__':
    main()
