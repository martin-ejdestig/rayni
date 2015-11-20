/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2015 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_TIME_STOPWATCH_H_
#define _RAYNI_LIB_TIME_STOPWATCH_H_

#include <chrono>
#include <string>

namespace Rayni
{
	class Stopwatch
	{
	public:
		using clock = std::chrono::high_resolution_clock;
		using time_point = clock::time_point;
		using duration = clock::duration;

		void start()
		{
			start(clock::now());
		}

		void stop()
		{
			stop(clock::now());
		}

		void start(time_point time_point);
		void stop(time_point time_point);

		bool is_started() const
		{
			return started;
		}

		duration get_duration() const;

	private:
		bool started = false;
		time_point time_start;
		time_point time_end;
	};
}

#endif // _RAYNI_LIB_TIME_STOPWATCH_H_
