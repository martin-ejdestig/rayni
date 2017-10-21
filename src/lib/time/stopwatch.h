/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2017 Martin Ejdestig <marejde@gmail.com>
 *
 * Rayni is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rayni is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rayni. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RAYNI_LIB_TIME_STOPWATCH_H
#define RAYNI_LIB_TIME_STOPWATCH_H

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

		void stop()
		{
			stop(clock::now());
		}

		void start(clock::time_point time_point);
		void stop(clock::time_point time_point);

		bool is_started() const
		{
			return started_;
		}

		clock::duration duration() const;

	private:
		bool started_ = false;
		clock::time_point time_start_;
		clock::time_point time_end_;
	};
}

#endif // RAYNI_LIB_TIME_STOPWATCH_H
