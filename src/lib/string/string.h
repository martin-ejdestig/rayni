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

#ifndef _RAYNI_LIB_STRING_STRING_H_
#define _RAYNI_LIB_STRING_STRING_H_

#include <experimental/optional>
#include <string>

namespace Rayni
{
	std::string string_center(std::string::size_type width, const std::string &str);
	std::string string_right_align(std::string::size_type width, const std::string &str);

	std::string string_to_lower(const std::string &str);

	std::experimental::optional<float> string_to_float(const std::string &str);
	std::experimental::optional<double> string_to_double(const std::string &str);

	template <typename T>
	std::experimental::optional<T> string_to_number(const std::string &str);

	template <>
	inline std::experimental::optional<float> string_to_number(const std::string &str)
	{
		return string_to_float(str);
	}

	template <>
	inline std::experimental::optional<double> string_to_number(const std::string &str)
	{
		return string_to_double(str);
	}
}

#endif // _RAYNI_LIB_STRING_STRING_H_
