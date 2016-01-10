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

#include <gtest/gtest.h>

#include "lib/math/math.h"
#include "unit_tests/lib/expect_real_eq.h"

namespace Rayni
{
	TEST(MathTest, Lerp)
	{
		EXPECT_REAL_EQ(5.0, lerp(-0.5, 10, 20));
		EXPECT_REAL_EQ(10.0, lerp(0.0, 10, 20));
		EXPECT_REAL_EQ(15.0, lerp(0.5, 10, 20));
		EXPECT_REAL_EQ(20.0, lerp(1.0, 10, 20));
		EXPECT_REAL_EQ(25.0, lerp(1.5, 10, 20));
	}

	TEST(MathTest, Blerp)
	{
		EXPECT_REAL_EQ(-5.0, blerp(-0.5, -0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(0.0, blerp(0.0, -0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(5.0, blerp(0.5, -0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(10.0, blerp(1.0, -0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(15.0, blerp(1.5, -0.5, 10, 20, 30, 40));

		EXPECT_REAL_EQ(5.0, blerp(-0.5, 0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(10.0, blerp(0.0, 0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(15.0, blerp(0.5, 0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(20.0, blerp(1.0, 0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(25.0, blerp(1.5, 0, 10, 20, 30, 40));

		EXPECT_REAL_EQ(15.0, blerp(-0.5, 0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(20.0, blerp(0.0, 0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(25.0, blerp(0.5, 0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(30.0, blerp(1.0, 0.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(35.0, blerp(1.5, 0.5, 10, 20, 30, 40));

		EXPECT_REAL_EQ(25.0, blerp(-0.5, 1.0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(30.0, blerp(0.0, 1.0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(35.0, blerp(0.5, 1.0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(40.0, blerp(1.0, 1.0, 10, 20, 30, 40));
		EXPECT_REAL_EQ(45.0, blerp(1.5, 1.0, 10, 20, 30, 40));

		EXPECT_REAL_EQ(35.0, blerp(-0.5, 1.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(40.0, blerp(0.0, 1.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(45.0, blerp(0.5, 1.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(50.0, blerp(1.0, 1.5, 10, 20, 30, 40));
		EXPECT_REAL_EQ(55.0, blerp(1.5, 1.5, 10, 20, 30, 40));
	}

	TEST(MathTest, Frac)
	{
		EXPECT_REAL_EQ(0.1, frac(-1.9));
		EXPECT_REAL_EQ(0.1, frac(-0.9));
		EXPECT_REAL_EQ(0.1, frac(0.1));
		EXPECT_REAL_EQ(0.1, frac(1.1));
	}

	TEST(MathTest, Ifloor)
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

	TEST(MathTest, RadiansFromDegrees)
	{
		for (int i = -8; i <= 8; i++)
			EXPECT_REAL_EQ(2 * PI * i / 8, radians_from_degrees(360 * i / 8)) << "i: " << i;
	}
}
