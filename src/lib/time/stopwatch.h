// This file is part of Rayni.
//
// Copyright (C) 2014-2020 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_TIME_STOPWATCH_H
#define RAYNI_LIB_TIME_STOPWATCH_H

#include <cassert>
#include <chrono>
#include <string>

namespace Rayni
{
	class Stopwatch
	{
	public:
		using clock = std::chrono::steady_clock;

		static_assert(clock::is_steady, "Stopwatch clock must be steady");

		void start()
		{
			start(clock::now());
		}

		void start(clock::time_point time_point)
		{
			started_ = true;
			time_start_ = time_point;
			time_end_ = time_start_;
		}

		void stop()
		{
			stop(clock::now());
		}

		void stop(clock::time_point time_point)
		{
			assert(started_ && time_start_ <= time_point);
			started_ = false;
			time_end_ = time_point;
		}

		bool is_started() const
		{
			return started_;
		}

		clock::duration duration() const
		{
			return (started_ ? clock::now() : time_end_) - time_start_;
		}

	private:
		bool started_ = false;
		clock::time_point time_start_;
		clock::time_point time_end_;
	};
}

#endif // RAYNI_LIB_TIME_STOPWATCH_H
