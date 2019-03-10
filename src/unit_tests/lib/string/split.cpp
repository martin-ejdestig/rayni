// This file is part of Rayni.
//
// Copyright (C) 2018-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/split.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	using Result = std::vector<std::string>;

	template <std::size_t N>
	using ResultArray = std::array<std::string, N>;
}

namespace Rayni
{
	TEST(StringSplit, EmptyString)
	{
		EXPECT_EQ(Result({}), string_split("", ' '));
	}

	TEST(StringSplit, Basic)
	{
		EXPECT_EQ(Result({"abc"}), string_split("abc", ' '));
		EXPECT_EQ(Result({"abc", "de"}), string_split("abc de", ' '));
		EXPECT_EQ(Result({"abc", "de", "fgh"}), string_split("abc de fgh", ' '));
		EXPECT_EQ(Result({"abc", "de", "fgh", "ijkl"}), string_split("abc de fgh ijkl", ' '));
		EXPECT_EQ(Result({"abc", "de", "fgh", "ijkl", "m"}), string_split("abc de fgh ijkl m", ' '));
		EXPECT_EQ(Result({"abc", "de", "fgh", "ijkl", "m", "no"}), string_split("abc de fgh ijkl m no", ' '));
	}

	TEST(StringSplit, SeparatorOtherThanSpace)
	{
		EXPECT_EQ(Result({"abc_def_ghi"}), string_split("abc_def_ghi", ' '));
		EXPECT_EQ(Result({"abc", "def", "ghi"}), string_split("abc_def_ghi", '_'));
	}

	TEST(StringSplit, StringContainsSeparatorOnly)
	{
		EXPECT_EQ(Result({"", ""}), string_split(" ", ' '));
		EXPECT_EQ(Result({"", "", ""}), string_split("  ", ' '));
		EXPECT_EQ(Result({"", "", "", ""}), string_split("   ", ' '));
		EXPECT_EQ(Result({"", "", "", "", ""}), string_split("    ", ' '));
	}

	TEST(StringSplit, ConsecutiveSeparatorsAtEnd)
	{
		EXPECT_EQ(Result({"abc", "def", ""}), string_split("abc def ", ' '));
		EXPECT_EQ(Result({"abc", "def", "", ""}), string_split("abc def  ", ' '));
		EXPECT_EQ(Result({"abc", "def", "", "", ""}), string_split("abc def   ", ' '));
		EXPECT_EQ(Result({"abc", "def", "", "", "", ""}), string_split("abc def    ", ' '));
	}

	TEST(StringSplit, ConsecutiveSeparatorsAtStart)
	{
		EXPECT_EQ(Result({"", "abc", "def"}), string_split(" abc def", ' '));
		EXPECT_EQ(Result({"", "", "abc", "def"}), string_split("  abc def", ' '));
		EXPECT_EQ(Result({"", "", "", "abc", "def"}), string_split("   abc def", ' '));
	}

	TEST(StringSplit, ConsecutiveSeparatorsInMiddle)
	{
		EXPECT_EQ(Result({"abc", "", "def"}), string_split("abc  def", ' '));
		EXPECT_EQ(Result({"abc", "", "", "def"}), string_split("abc   def", ' '));
		EXPECT_EQ(Result({"abc", "", "", "", "def"}), string_split("abc    def", ' '));
	}

	TEST(StringSplit, ToArrayEmptyString)
	{
		EXPECT_EQ(ResultArray<0>({}), (string_split_to_array<std::string, 0>("", ' ')));
		EXPECT_EQ(ResultArray<1>({""}), (string_split_to_array<std::string, 1>("", ' ')));
		EXPECT_EQ(ResultArray<2>({"", ""}), (string_split_to_array<std::string, 2>("", ' ')));
	}

	TEST(StringSplit, ToArrayBasic)
	{
		EXPECT_EQ(ResultArray<1>({"abc"}), (string_split_to_array<std::string, 1>("abc", ' ')));
		EXPECT_EQ(ResultArray<2>({"abc", "de"}), (string_split_to_array<std::string, 2>("abc de", ' ')));
		EXPECT_EQ(ResultArray<3>({"abc", "de", "fgh"}),
		          (string_split_to_array<std::string, 3>("abc de fgh", ' ')));
	}

	TEST(StringSplit, ToArraySeparatorOtherThanSpace)
	{
		EXPECT_EQ(ResultArray<3>({"abc", "def", "ghi"}),
		          (string_split_to_array<std::string, 3>("abc_def_ghi", '_')));
	}

	TEST(StringSplit, ToArrayStringContainsSeparatorOnly)
	{
		EXPECT_EQ(ResultArray<2>({"", ""}), (string_split_to_array<std::string, 2>(" ", ' ')));

		EXPECT_EQ(ResultArray<3>({"", "", ""}), (string_split_to_array<std::string, 3>("  ", ' ')));

		EXPECT_EQ(ResultArray<4>({"", "", "", ""}), (string_split_to_array<std::string, 4>("   ", ' ')));

		EXPECT_EQ(ResultArray<5>({"", "", "", "", ""}), (string_split_to_array<std::string, 5>("    ", ' ')));
	}

	TEST(StringSplit, ToArrayConsecutiveSeparatorsAtEnd)
	{
		EXPECT_EQ(ResultArray<2>({"abc", "def "}), (string_split_to_array<std::string, 2>("abc def ", ' ')));
		EXPECT_EQ(ResultArray<3>({"abc", "def", ""}), (string_split_to_array<std::string, 3>("abc def ", ' ')));

		EXPECT_EQ(ResultArray<3>({"abc", "def", " "}),
		          (string_split_to_array<std::string, 3>("abc def  ", ' ')));
		EXPECT_EQ(ResultArray<4>({"abc", "def", "", ""}),
		          (string_split_to_array<std::string, 4>("abc def  ", ' ')));

		EXPECT_EQ(ResultArray<4>({"abc", "def", "", " "}),
		          (string_split_to_array<std::string, 4>("abc def   ", ' ')));
		EXPECT_EQ(ResultArray<5>({"abc", "def", "", "", ""}),
		          (string_split_to_array<std::string, 5>("abc def   ", ' ')));

		EXPECT_EQ(ResultArray<5>({"abc", "def", "", "", " "}),
		          (string_split_to_array<std::string, 5>("abc def    ", ' ')));
		EXPECT_EQ(ResultArray<6>({"abc", "def", "", "", "", ""}),
		          (string_split_to_array<std::string, 6>("abc def    ", ' ')));
	}

	TEST(StringSplit, ToArrayConsecutiveSeparatorsAtStart)
	{
		EXPECT_EQ(ResultArray<3>({"", "abc", "def"}), (string_split_to_array<std::string, 3>(" abc def", ' ')));

		EXPECT_EQ(ResultArray<4>({"", "", "abc", "def"}),
		          (string_split_to_array<std::string, 4>("  abc def", ' ')));

		EXPECT_EQ(ResultArray<5>({"", "", "", "abc", "def"}),
		          (string_split_to_array<std::string, 5>("   abc def", ' ')));
	}

	TEST(StringSplit, ToArrayConsecutiveSeparatorsInMiddle)
	{
		EXPECT_EQ(ResultArray<3>({"abc", "", "def"}), (string_split_to_array<std::string, 3>("abc  def", ' ')));

		EXPECT_EQ(ResultArray<4>({"abc", "", "", "def"}),
		          (string_split_to_array<std::string, 4>("abc   def", ' ')));

		EXPECT_EQ(ResultArray<5>({"abc", "", "", "", "def"}),
		          (string_split_to_array<std::string, 5>("abc    def", ' ')));
	}

	TEST(StringSplit, ToArrayTooSmallLeavesRemainingStringInLast)
	{
		EXPECT_EQ(ResultArray<1>({"abc def ghi"}), (string_split_to_array<std::string, 1>("abc def ghi", ' ')));

		EXPECT_EQ(ResultArray<2>({"abc", "def ghi"}),
		          (string_split_to_array<std::string, 2>("abc def ghi", ' ')));
	}

	TEST(StringSplit, ToArrayTooLargeLeavesEndElementsEmpty)
	{
		EXPECT_EQ(ResultArray<2>({"abc", ""}), (string_split_to_array<std::string, 2>("abc", ' ')));

		EXPECT_EQ(ResultArray<3>({"abc", "", ""}), (string_split_to_array<std::string, 3>("abc", ' ')));

		EXPECT_EQ(ResultArray<3>({"abc", "def", ""}), (string_split_to_array<std::string, 3>("abc def", ' ')));

		EXPECT_EQ(ResultArray<4>({"abc", "def", "", ""}),
		          (string_split_to_array<std::string, 4>("abc def", ' ')));

		EXPECT_EQ(ResultArray<4>({"abc", "def", "ghi", ""}),
		          (string_split_to_array<std::string, 4>("abc def ghi", ' ')));

		EXPECT_EQ(ResultArray<5>({"abc", "def", "ghi", "", ""}),
		          (string_split_to_array<std::string, 5>("abc def ghi", ' ')));
	}

#if !defined __clang__
	// TODO: Does currently not compile with latest Clang. Fixed yet?
	TEST(StringSplit, ToArrayConstexpr)
	{
		static constexpr std::array<std::string_view, 4> SPLITS =
		        string_split_to_array<std::string_view, 4>("abc def ghi jkl", ' ');

		static constexpr std::array<std::string_view, 4> EXPECTED_SPLITS = {"abc", "def", "ghi", "jkl"};

		EXPECT_EQ(EXPECTED_SPLITS, SPLITS);
	}
#endif
}
