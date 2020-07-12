// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
//
// Rayni is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Rayni is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rayni. If not, see <http://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lib/string/duration_format.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace Rayni
{
	std::string duration_format(std::chrono::nanoseconds ns, const DurationFormatOptions &options)
	{
		using namespace std::chrono_literals;

		auto hh = std::chrono::duration_cast<std::chrono::hours>(ns).count();
		auto mm = std::chrono::duration_cast<std::chrono::minutes>(ns % 1h).count();
		auto ss = std::chrono::duration_cast<std::chrono::duration<double>>(ns % 1min).count();
		std::ostringstream stream;

		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 1h)
			stream << hh << ":";

		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 1min)
			stream << std::setw(2) << std::setfill('0') << mm << ":";

		int seconds_width = 1;
		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 10s)
			seconds_width++;
		if (options.seconds_precision > 0)
			seconds_width++;
		seconds_width += options.seconds_precision;

		stream << std::setw(seconds_width) << std::setfill('0') << std::fixed
		       << std::setprecision(options.seconds_precision) << (options.floor_seconds ? std::floor(ss) : ss);

		return stream.str();
	}
}
