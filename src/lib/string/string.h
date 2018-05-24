/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_STRING_STRING_H
#define RAYNI_LIB_STRING_STRING_H

#include <cassert>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

namespace Rayni
{
	std::string string_center(std::string::size_type width, const std::string &str);
	std::string string_right_align(std::string::size_type width, const std::string &str);

	std::string string_to_lower(const std::string &str);

	std::optional<float> string_to_float(const std::string &str);
	std::optional<double> string_to_double(const std::string &str);
	std::optional<long> string_to_long(const std::string &str);

	template <typename T>
	std::optional<T> string_to_number(const std::string &str)
	{
		if constexpr (std::is_same_v<T, float>)
			return string_to_float(str);

		if constexpr (std::is_same_v<T, double>)
			return string_to_double(str);

		// TODO: Can be smarter here, use more SFINAE etc. But good enough for now (only
		//       need <= std::uint32_t and long is 64 bits on all interesting platforms).
		if constexpr (std::is_integral_v<T> && sizeof(long) > sizeof(T))
		{
			std::optional<long> l = string_to_long(str);

			if (!l || *l < std::numeric_limits<T>::lowest() || *l > std::numeric_limits<T>::max())
				return std::nullopt;

			return static_cast<T>(*l);
		}

		assert(false);

		return std::nullopt;
	}
}

#endif // RAYNI_LIB_STRING_STRING_H
