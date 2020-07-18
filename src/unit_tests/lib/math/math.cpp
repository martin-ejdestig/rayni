// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/math.h"

#include <gtest/gtest.h>

#include <cmath>

namespace Rayni
{
	namespace
	{
		void test_next_up(real_t r)
		{
			EXPECT_EQ(real_next_up(r), std::nextafter(r, REAL_INFINITY)) << "r = " << r;
		}

		void test_next_down(real_t r)
		{
			EXPECT_EQ(real_next_down(r), std::nextafter(r, -REAL_INFINITY)) << "r = " << r;
		}
	}

	TEST(Math, Frac)
	{
		EXPECT_NEAR(0.1, frac(-1.9), 1e-7);
		EXPECT_NEAR(0.1, frac(-0.9), 1e-7);
		EXPECT_NEAR(0.1, frac(0.1), 1e-7);
		EXPECT_NEAR(0.1, frac(1.1), 1e-7);
	}

	TEST(Math, Ifloor)
	{
		EXPECT_EQ(-2, ifloor(-1.1));
		EXPECT_EQ(-1, ifloor(-0.9));
		EXPECT_EQ(-1, ifloor(-0.1));
		EXPECT_EQ(0, ifloor(0.1));
		EXPECT_EQ(0, ifloor(0.9));
		EXPECT_EQ(1, ifloor(1.1));
		EXPECT_EQ(1, ifloor(1.9));
		EXPECT_EQ(2, ifloor(2.1));
	}

	TEST(Math, RadiansFromDegrees)
	{
		for (int i = -8; i <= 8; i++)
			EXPECT_NEAR(2 * PI * i / 8, radians_from_degrees(real_t(360) * i / 8), 1e-6) << "i: " << i;
	}

	TEST(Math, RealNextUp)
	{
		test_next_up(0);
		test_next_up(-0);

		test_next_up(1);
		test_next_up(-1);

		real_t two_plus_ulp = std::nextafter(real_t(2), REAL_INFINITY);
		test_next_up(two_plus_ulp);
		test_next_up(-two_plus_ulp);

		real_t two_minus_ulp = std::nextafter(real_t(2), -REAL_INFINITY);
		test_next_up(two_minus_ulp);
		test_next_up(-two_minus_ulp);

		test_next_up(REAL_MAX);
		test_next_up(REAL_LOWEST);

		test_next_up(REAL_INFINITY);
		test_next_up(-REAL_INFINITY);
	}

	TEST(Math, RealNextDown)
	{
		test_next_down(0);
		test_next_down(-0);

		test_next_down(1);
		test_next_down(-1);

		real_t two_plus_ulp = std::nextafter(real_t(2), REAL_INFINITY);
		test_next_down(two_plus_ulp);
		test_next_down(-two_plus_ulp);

		real_t two_minus_ulp = std::nextafter(real_t(2), -REAL_INFINITY);
		test_next_down(two_minus_ulp);
		test_next_down(-two_minus_ulp);

		test_next_down(REAL_MAX);
		test_next_down(REAL_LOWEST);

		test_next_down(REAL_INFINITY);
		test_next_down(-REAL_INFINITY);
	}
}
