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

#include "lib/math/matrix3x3.h"
#include "lib/math/quaternion.h"

namespace Rayni
{
	TEST(Matrix3x3Test, OperatorIndexing)
	{
		Matrix3x3 m({1, 2, 3}, {4, 5, 6}, {7, 8, 9});
		EXPECT_NEAR(1, m(0, 0), 1e-100);
		EXPECT_NEAR(2, m(0, 1), 1e-100);
		EXPECT_NEAR(3, m(0, 2), 1e-100);
		EXPECT_NEAR(4, m(1, 0), 1e-100);
		EXPECT_NEAR(5, m(1, 1), 1e-100);
		EXPECT_NEAR(6, m(1, 2), 1e-100);
		EXPECT_NEAR(7, m(2, 0), 1e-100);
		EXPECT_NEAR(8, m(2, 1), 1e-100);
		EXPECT_NEAR(9, m(2, 2), 1e-100);

		const Matrix3x3 mc({1, 2, 3}, {4, 5, 6}, {7, 8, 9});
		EXPECT_NEAR(1, mc(0, 0), 1e-100);
		EXPECT_NEAR(2, mc(0, 1), 1e-100);
		EXPECT_NEAR(3, mc(0, 2), 1e-100);
		EXPECT_NEAR(4, mc(1, 0), 1e-100);
		EXPECT_NEAR(5, mc(1, 1), 1e-100);
		EXPECT_NEAR(6, mc(1, 2), 1e-100);
		EXPECT_NEAR(7, mc(2, 0), 1e-100);
		EXPECT_NEAR(8, mc(2, 1), 1e-100);
		EXPECT_NEAR(9, mc(2, 2), 1e-100);
	}

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

	TEST(Matrix3x3Test, MaxAbsoluteRowSumNorm)
	{
		EXPECT_NEAR(60, Matrix3x3({10, 20, 30}, {1, 2, 3}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({-10, -20, -30}, {1, 2, 3}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);

		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {10, 20, 30}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {-10, -20, -30}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);

		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {4, 5, 6}, {10, 20, 30}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {4, 5, 6}, {-10, -20, -30}).max_absolute_row_sum_norm(), 1e-100);
	}

	TEST(Matrix3x3Test, Rotation)
	{
		Quaternion q = Matrix3x3({0.133337, -0.666669, 0.733333},
		                         {0.933331, 0.333342, 0.133338},
		                         {-0.333344, 0.666662, 0.666667})
		                       .rotation();

		EXPECT_NEAR(0.18257, q.x(), 1e-5);
		EXPECT_NEAR(0.36515, q.y(), 1e-5);
		EXPECT_NEAR(0.54772, q.z(), 1e-5);
		EXPECT_NEAR(0.73030, q.w(), 1e-5);
	}
}
