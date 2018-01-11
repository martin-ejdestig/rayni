/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2018 Martin Ejdestig <marejde@gmail.com>
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
	void Stopwatch::start(clock::time_point time_point)
	{
		started_ = true;
		time_start_ = time_point;
		time_end_ = time_start_;
	}

	void Stopwatch::stop(clock::time_point time_point)
	{
		assert(started_ && time_start_ <= time_point);
		started_ = false;
		time_end_ = time_point;
	}

	Stopwatch::clock::duration Stopwatch::duration() const
	{
		return (started_ ? clock::now() : time_end_) - time_start_;
	}
}
