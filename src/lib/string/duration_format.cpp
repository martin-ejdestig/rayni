// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/string.h"

// TODO: Remove once https://bugs.llvm.org/show_bug.cgi?id=44325 is fixed.
// Spurious warning when e.g. >= is rewritten to <=>, which happens with chrono. *sigh*
#if defined __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

namespace Rayni
{
	std::string duration_format(std::chrono::nanoseconds ns, const DurationFormatOptions &options)
	{
		using namespace std::chrono_literals;

		auto hh = std::chrono::duration_cast<std::chrono::hours>(ns).count();
		auto mm = std::chrono::duration_cast<std::chrono::minutes>(ns % 1h).count();
		auto ss = std::chrono::duration_cast<std::chrono::duration<double>>(ns % 1min).count();
		std::string str;

		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 1h)
			str += string_printf("%d:", int(hh));

		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 1min)
			str += string_printf("%02d:", int(mm));

		int seconds_width = 1;
		// NOLINTNEXTLINE(modernize-use-nullptr) TODO: https://bugs.llvm.org/show_bug.cgi?id=46235 fixed?
		if (ns >= 10s)
			seconds_width++;
		if (options.seconds_precision > 0)
			seconds_width++;
		seconds_width += options.seconds_precision;

		str += string_printf("%0*.*f",
		                     seconds_width,
		                     int(options.seconds_precision),
		                     options.floor_seconds ? std::floor(ss) : ss);

		if (options.seconds_precision > 0) { // Hack to ignore locale specific decimal point.
			char *c = &str[str.size() - options.seconds_precision - 1];
			if (*c != '.')
				*c = '.';
		}

		return str;
	}
}

#if defined __clang__
#	pragma clang diagnostic pop
#endif
