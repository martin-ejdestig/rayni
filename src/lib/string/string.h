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

#ifndef RAYNI_LIB_STRING_STRING_H
#define RAYNI_LIB_STRING_STRING_H

#include <cassert>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace Rayni
{
	std::string string_center(std::string::size_type width, const std::string &str);
	std::string string_right_align(std::string::size_type width, const std::string &str);

	std::string string_to_lower(const std::string &str);

	// TODO: Remove string_to_float/double() once std::from_chars() is fully supported in
	// libstdc++ (currently only supports int:s) and libc++ (currently not implemented at all).
	//
	// Modify string_to_number() to unconditionally use std::from_chars(). Still want to keep
	// string_to_number() to not repeat std::from_chars() begin/and argument creation and
	// std::from_chars_result error checking everywhere.
	std::optional<float> string_to_float(std::string_view str);
	std::optional<double> string_to_double(std::string_view str);

	template <typename T>
	std::optional<T> string_to_number(std::string_view str)
	{
		if constexpr (std::is_same_v<T, float>)
			return string_to_float(str);

		if constexpr (std::is_same_v<T, double>)
			return string_to_double(str);

		if constexpr (std::is_integral_v<T>)
		{
			T value;
			std::from_chars_result result = std::from_chars(str.data(), str.data() + str.length(), value);

			if (result.ec != std::errc() || result.ptr != str.data() + str.length())
				return std::nullopt;

			return value;
		}

		assert(false);

		return std::nullopt;
	}
}

#endif // RAYNI_LIB_STRING_STRING_H
