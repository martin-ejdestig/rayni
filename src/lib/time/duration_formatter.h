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

#ifndef _RAYNI_LIB_TIME_DURATION_FORMATTER_H_
#define _RAYNI_LIB_TIME_DURATION_FORMATTER_H_

#include <chrono>
#include <ios>
#include <string>

namespace Rayni
{
	class DurationFormatter
	{
	public:
		DurationFormatter &set_seconds_precision(std::streamsize seconds_precision)
		{
			this->seconds_precision = seconds_precision;
			return *this;
		}

		DurationFormatter &set_floor_seconds(bool floor_seconds)
		{
			this->floor_seconds = floor_seconds;
			return *this;
		}

		template <typename Duration>
		std::string format(Duration duration) const
		{
			using namespace std::chrono_literals;
			auto hh = std::chrono::duration_cast<std::chrono::hours>(duration).count();
			auto mm = std::chrono::duration_cast<std::chrono::minutes>(duration % 1h).count();
			auto ss = std::chrono::duration_cast<std::chrono::duration<float>>(duration % 1min).count();

			return format(hh, mm, ss);
		}

	private:
		std::string format(unsigned int hh, unsigned int mm, float ss) const;

		std::streamsize seconds_precision = 0;
		bool floor_seconds = false;
	};
}

#endif // _RAYNI_LIB_TIME_DURATION_FORMATTER_H_