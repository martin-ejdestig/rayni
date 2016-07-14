/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2016 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/time/scoped_stopwatch.h"

#include <ostream>

#include "lib/time/duration_formatter.h"

namespace Rayni
{
	void ScopedStopwatch::print_result() const
	{
		std::string duration = DurationFormatter().set_seconds_precision(6).format(stopwatch.duration());
		ostream << prefix << ": " << duration << '\n';
	}
}
