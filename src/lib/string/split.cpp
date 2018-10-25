/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/split.h"

#include <string>
#include <string_view>
#include <vector>

namespace Rayni
{
	std::vector<std::string> string_split(std::string_view string, char split_char)
	{
		std::vector<std::string> splits;
		std::string_view remaining = string;

		while (!remaining.empty())
		{
			auto [left, right] = string_split_to_array<std::string_view, 2>(remaining, split_char);
			splits.emplace_back(left);
			remaining = right;
		}

		// An empty string is considered to be on the right of a trailing separator. It is
		// what Python's str.split() does and also aligns more with string_split_to_array().
		if (!string.empty() && string.back() == split_char)
			splits.emplace_back();

		return splits;
	}
}
