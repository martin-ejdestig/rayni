/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016 Martin Ejdestig <marejde@gmail.com>
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

#include <cmath>
#include <string>

#include "lib/math/math.h"
#include "lib/math/matrix3x3.h"

namespace Rayni
{
	TEST(Matrix3x3Test, Trace)
	{
		EXPECT_NEAR(14, Matrix3x3({2, 100, 100}, {100, 4, 100}, {100, 100, 8}).trace(), 1e-100);
	}

	TEST(Matrix3x3Test, MaxDiagonalPosition)
	{
		EXPECT_EQ(0, Matrix3x3({3, 0, 0}, {0, 1, 0}, {0, 0, 2}).max_diagonal_position());
		EXPECT_EQ(1, Matrix3x3({2, 0, 0}, {0, 3, 0}, {0, 0, 1}).max_diagonal_position());
		EXPECT_EQ(2, Matrix3x3({1, 0, 0}, {0, 2, 0}, {0, 0, 3}).max_diagonal_position());
	}
}
