/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_TIME_SCOPED_STOPWATCH_H
#define RAYNI_LIB_TIME_SCOPED_STOPWATCH_H

#include <iostream>
#include <ostream>

#include "lib/time/stopwatch.h"

namespace Rayni
{
	class ScopedStopwatch
	{
	public:
		explicit ScopedStopwatch(const std::string &prefix) : ScopedStopwatch(prefix, std::cout)
		{
		}

		ScopedStopwatch(const std::string &prefix, std::ostream &ostream) : prefix_(prefix), ostream_(ostream)
		{
			stopwatch_.start();
		}

		ScopedStopwatch(const ScopedStopwatch &other) = delete;
		ScopedStopwatch(ScopedStopwatch &&other) = delete;

		~ScopedStopwatch()
		{
			stopwatch_.stop();
			print_result();
		}

		ScopedStopwatch &operator=(const ScopedStopwatch &other) = delete;
		ScopedStopwatch &operator=(ScopedStopwatch &&other) = delete;

	private:
		void print_result() const;

		const std::string prefix_;
		std::ostream &ostream_;

		Stopwatch stopwatch_;
	};
}

#endif // RAYNI_LIB_TIME_SCOPED_STOPWATCH_H
