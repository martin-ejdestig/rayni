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

import sys

class ProgressPrinter:
    def __init__(self):
        self._info_string = ''
        self._count = 0
        self._done_count = 0

    def start(self, info_string, done_count):
        self._info_string = info_string
        self._count = 0
        self._done_count = done_count
        self._print()

    def result(self, result_string):
        self._count += 1
        if result_string:
            sys.stdout.write('\r' + result_string + '\n')
        self._print()

    def _print(self):
        trailing_str = '. Done.\n' if self._count == self._done_count else '...'
        sys.stdout.write('\r[{0}/{1}] {2}{3}'.format(self._count,
                                                     self._done_count,
                                                     self._info_string,
                                                     trailing_str))
        sys.stdout.flush()
