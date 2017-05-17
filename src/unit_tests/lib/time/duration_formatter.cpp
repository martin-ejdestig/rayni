/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <gtest/gtest.h>

#include <chrono>

#include "lib/time/duration_formatter.h"

namespace Rayni
{
	TEST(DurationFormatter, Format)
	{
		// TODO: Remove NOLINT when https://llvm.org/bugs/show_bug.cgi?id=25594 is fixed.
		using namespace std::chrono_literals; // NOLINT
		DurationFormatter formatter;

		EXPECT_EQ("00", formatter.format(0s));
		EXPECT_EQ("01", formatter.format(1s));
		EXPECT_EQ("01:00", formatter.format(1min));
		EXPECT_EQ("1:00:00", formatter.format(1h));
		EXPECT_EQ("12:34:56", formatter.format(12h + 34min + 56s));

		formatter.set_seconds_precision(2);
		EXPECT_EQ("00.00", formatter.format(0s));
		EXPECT_EQ("00.09", formatter.format(90ms));
		EXPECT_EQ("00.10", formatter.format(99ms));
		EXPECT_EQ("00.12", formatter.format(123ms));
		EXPECT_EQ("05.00", formatter.format(5s));
		EXPECT_EQ("20.00", formatter.format(20s));
		EXPECT_EQ("34.50", formatter.format(34s + 499ms));
		EXPECT_EQ("34.50", formatter.format(34s + 500ms));
		EXPECT_EQ("34.50", formatter.format(34s + 504ms));
		EXPECT_EQ("34.51", formatter.format(34s + 505ms));
		EXPECT_EQ("35.00", formatter.format(34s + 999ms));
		EXPECT_EQ("35.00", formatter.format(34s + 1001ms));

		formatter.set_floor_seconds(true);
		EXPECT_EQ("00.00", formatter.format(0s));
		EXPECT_EQ("00.00", formatter.format(999ms));
		EXPECT_EQ("01.00", formatter.format(1s + 499ms));
		EXPECT_EQ("01.00", formatter.format(1s + 999ms));
		EXPECT_EQ("02.00", formatter.format(1s + 1001ms));
	}
}
