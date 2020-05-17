// This file is part of Rayni.
//
// Copyright (C) 2018-2020 Martin Ejdestig <marejde@gmail.com>
//
// Rayni is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Rayni is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rayni. If not, see <http://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RAYNI_LIB_STRING_SPLIT_H
#define RAYNI_LIB_STRING_SPLIT_H

#include <array>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Rayni
{
	std::vector<std::string> string_split(std::string_view string, char split_char);

	template <typename T, std::size_t N>
	constexpr std::array<T, N> string_split_to_array(std::string_view string, char split_char)
	{
		static_assert(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>);

		std::array<T, N> splits;
		std::string_view remaining = string;

		for (std::size_t i = 0; i < N && !remaining.empty(); i++)
		{
			std::size_t pos = i < N - 1 ? remaining.find(split_char) : std::string_view::npos;

			if (pos == std::string_view::npos)
			{
				splits[i] = remaining;
				break;
			}

			splits[i] = std::string_view(remaining.data(), pos);
			remaining = std::string_view(remaining.data() + pos + 1, remaining.length() - pos - 1);
		}

		return splits;
	}
}

#endif // RAYNI_LIB_STRING_SPLIT_H
