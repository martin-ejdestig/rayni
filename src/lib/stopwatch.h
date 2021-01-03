// This file is part of Rayni.
//
// Copyright (C) 2014-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_STOPWATCH_H
#define RAYNI_LIB_STOPWATCH_H

#include <cassert>
#include <chrono>
#include <string>

#include "lib/string/duration_format.h"

namespace Rayni
{
	class Stopwatch
	{
	public:
		using clock = std::chrono::steady_clock;

		static_assert(clock::is_steady, "Stopwatch clock must be steady");

		Stopwatch &start()
		{
			return start(clock::now());
		}

		Stopwatch &start(clock::time_point time_point)
		{
			started_ = true;
			time_start_ = time_point;
			time_end_ = time_start_;
			return *this;
		}

		Stopwatch &stop()
		{
			return stop(clock::now());
		}

		Stopwatch &stop(clock::time_point time_point)
		{
			assert(started_ && time_start_ <= time_point);
			started_ = false;
			time_end_ = time_point;
			return *this;
		}

		bool started() const
		{
			return started_;
		}

		clock::duration duration() const
		{
			return (started_ ? clock::now() : time_end_) - time_start_;
		}

		std::string string(const DurationFormatOptions &options = {.seconds_precision = 3}) const
		{
			return duration_format(duration(), options);
		}

	private:
		bool started_ = false;
		clock::time_point time_start_;
		clock::time_point time_end_;
	};
}

#endif // RAYNI_LIB_STOPWATCH_H
