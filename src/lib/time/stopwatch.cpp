/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2016 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/time/stopwatch.h"

#include <cassert>

namespace Rayni
{
	void Stopwatch::start(time_point time_point)
	{
		started = true;
		time_start = time_point;
		time_end = time_start;
	}

	void Stopwatch::stop(time_point time_point)
	{
		assert(started && time_start <= time_point);
		started = false;
		time_end = time_point;
	}

	Stopwatch::duration Stopwatch::get_duration() const
	{
		return (started ? clock::now() : time_end) - time_start;
	}
}
