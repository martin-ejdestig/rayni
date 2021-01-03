// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/duration_format.h"

#include <gtest/gtest.h>

#include <chrono>

namespace
{
	using namespace std::chrono_literals;
}

namespace Rayni
{
	TEST(DurationFormat, Default)
	{
		EXPECT_EQ("0", duration_format(0s));
		EXPECT_EQ("1", duration_format(1s));
		EXPECT_EQ("01:00", duration_format(1min));
		EXPECT_EQ("1:00:00", duration_format(1h));
		EXPECT_EQ("12:34:56", duration_format(12h + 34min + 56s));
	}

	TEST(DurationFormat, SecondsPrecision)
	{
		EXPECT_EQ("0.00", duration_format(0s, {.seconds_precision = 2}));
		EXPECT_EQ("0.09", duration_format(90ms, {.seconds_precision = 2}));
		EXPECT_EQ("0.10", duration_format(99ms, {.seconds_precision = 2}));
		EXPECT_EQ("0.12", duration_format(123ms, {.seconds_precision = 2}));
		EXPECT_EQ("5.00", duration_format(5s, {.seconds_precision = 2}));
		EXPECT_EQ("20.00", duration_format(20s, {.seconds_precision = 2}));
		EXPECT_EQ("34.50", duration_format(34s + 499ms, {.seconds_precision = 2}));
		EXPECT_EQ("34.50", duration_format(34s + 500ms, {.seconds_precision = 2}));
		EXPECT_EQ("34.50", duration_format(34s + 504ms, {.seconds_precision = 2}));
		EXPECT_EQ("34.51", duration_format(34s + 505ms, {.seconds_precision = 2}));
		EXPECT_EQ("35.00", duration_format(34s + 999ms, {.seconds_precision = 2}));
		EXPECT_EQ("35.00", duration_format(34s + 1001ms, {.seconds_precision = 2}));
		EXPECT_EQ("01:35.00", duration_format(1min + 34s + 999ms, {.seconds_precision = 2}));
		EXPECT_EQ("01:35.00", duration_format(1min + 34s + 1001ms, {.seconds_precision = 2}));

		EXPECT_EQ("0.000", duration_format(0s, {.seconds_precision = 3}));
		EXPECT_EQ("0.090", duration_format(90ms, {.seconds_precision = 3}));
		EXPECT_EQ("0.099", duration_format(99ms, {.seconds_precision = 3}));
		EXPECT_EQ("0.123", duration_format(123ms, {.seconds_precision = 3}));
		EXPECT_EQ("5.000", duration_format(5s, {.seconds_precision = 3}));
		EXPECT_EQ("20.000", duration_format(20s, {.seconds_precision = 3}));
		EXPECT_EQ("34.499", duration_format(34s + 499ms, {.seconds_precision = 3}));
		EXPECT_EQ("34.500", duration_format(34s + 500ms, {.seconds_precision = 3}));
		EXPECT_EQ("34.504", duration_format(34s + 504ms, {.seconds_precision = 3}));
		EXPECT_EQ("34.505", duration_format(34s + 505ms, {.seconds_precision = 3}));
		EXPECT_EQ("34.999", duration_format(34s + 999ms, {.seconds_precision = 3}));
		EXPECT_EQ("35.001", duration_format(34s + 1001ms, {.seconds_precision = 3}));
		EXPECT_EQ("01:34.999", duration_format(1min + 34s + 999ms, {.seconds_precision = 3}));
		EXPECT_EQ("01:35.001", duration_format(1min + 34s + 1001ms, {.seconds_precision = 3}));
	}

	TEST(DurationFormat, FloorSeconds)
	{
		EXPECT_EQ("0.00", duration_format(0s, {.seconds_precision = 2, .floor_seconds = true}));
		EXPECT_EQ("0.00", duration_format(999ms, {.seconds_precision = 2, .floor_seconds = true}));
		EXPECT_EQ("1.00", duration_format(1s + 499ms, {.seconds_precision = 2, .floor_seconds = true}));
		EXPECT_EQ("1.00", duration_format(1s + 999ms, {.seconds_precision = 2, .floor_seconds = true}));
		EXPECT_EQ("2.00", duration_format(1s + 1001ms, {.seconds_precision = 2, .floor_seconds = true}));
	}
}
