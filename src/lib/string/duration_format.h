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

#ifndef RAYNI_LIB_STRING_DURATION_FORMAT_H
#define RAYNI_LIB_STRING_DURATION_FORMAT_H

#include <chrono>
#include <cstdint>
#include <string>

namespace Rayni
{
	struct DurationFormatOptions
	{
		std::uint8_t seconds_precision = 0;
		bool floor_seconds = false;
	};

	std::string duration_format(std::chrono::nanoseconds ns, const DurationFormatOptions &options = {});

	template <typename Duration>
	std::string duration_format(Duration duration, const DurationFormatOptions &options = {})
	{
		return duration_format(std::chrono::duration_cast<std::chrono::nanoseconds>(duration), options);
	}
}

#endif // RAYNI_LIB_STRING_DURATION_FORMAT_H
