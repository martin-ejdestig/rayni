/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_TIME_SCOPED_STOPWATCH_H_
#define _RAYNI_LIB_TIME_SCOPED_STOPWATCH_H_

#include <iostream>
#include <ostream>

#include "lib/time/stopwatch.h"

namespace Rayni
{
	class ScopedStopwatch
	{
	public:
		ScopedStopwatch(const std::string &prefix) : ScopedStopwatch(prefix, std::cout)
		{
		}

		ScopedStopwatch(const std::string &prefix, std::ostream &ostream) : prefix(prefix), ostream(ostream)
		{
			stopwatch.start();
		}

		~ScopedStopwatch()
		{
			stopwatch.stop();
			print_result();
		}

	private:
		void print_result() const;

		const std::string prefix;
		std::ostream &ostream;

		Stopwatch stopwatch;
	};
}

#endif // _RAYNI_LIB_TIME_SCOPED_STOPWATCH_H_
