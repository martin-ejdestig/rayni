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

#include "lib/string/string.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

namespace
{
	/**
	 * Thread local instance of std::istringstream with classic "C" locale.
	 *
	 * Used when parsing e.g. file formats where decimal point is a dot. The current locale,
	 * used by std::stof et al. and std::istringstream by default, may use a comma which will
	 * cause parsing to fail.
	 *
	 * Caching the instance has shown to be worthwhile since reading large object files may
	 * involve converting a lot of strings to floating point numbers. The time measurements
	 * below are from when loading a scene with the Stanford Buddah, Bunny and Dragon containing
	 * ~6 million number strings. Compiled with -Ofast -march=native, GCC 5.2.0,
	 * libstdc++5 3.3.6-5 and glibc 2.22-3.
	 *
	 * 3.3s - std::strtod (with system locale)
	 * 6.5s - std::istringstream (with system locale)
	 * 8.0s - std::istringstream + imbue
	 * 4.5s - std::istringstream + imbue + thread local reuse
	 */
	static std::istringstream &classic_locale_istringstream_get_with_string(const std::string &str)
	{
		static thread_local std::istringstream stream = [] {
			std::istringstream stream;
			stream.imbue(std::locale::classic());
			return stream;
		}();

		stream.clear();
		stream.str(str);

		return stream;
	}
}

namespace Rayni
{
	std::string string_center(std::string::size_type width, const std::string &str)
	{
		if (width <= str.length())
			return str;

		auto padding = width - str.length();
		auto padding_left = padding / 2;
		auto padding_right = padding - padding_left;

		return std::string(padding_left, ' ') + str + std::string(padding_right, ' ');
	}

	std::string string_right_align(std::string::size_type width, const std::string &str)
	{
		if (width <= str.length())
			return str;

		return std::string(width - str.length(), ' ') + str;
	}

	std::string string_to_lower(const std::string &str)
	{
		std::string result;
		std::transform(str.begin(), str.end(), std::back_inserter(result), [](int c) {
			return std::tolower(c);
		});
		return result;
	}

	std::experimental::optional<float> string_to_float(const std::string &str)
	{
		std::istringstream &stream = classic_locale_istringstream_get_with_string(str);
		float value;

		stream >> value;

		return stream.fail() || !stream.eof() ? std::experimental::nullopt :
		                                        std::experimental::make_optional(value);
	}

	std::experimental::optional<double> string_to_double(const std::string &str)
	{
		std::istringstream &stream = classic_locale_istringstream_get_with_string(str);
		double value;

		stream >> value;

		return stream.fail() || !stream.eof() ? std::experimental::nullopt :
		                                        std::experimental::make_optional(value);
	}
}
