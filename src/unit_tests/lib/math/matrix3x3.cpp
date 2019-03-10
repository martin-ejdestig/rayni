// This file is part of Rayni.
//
// Copyright (C) 2016-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/matrix3x3.h"

#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include "lib/math/math.h"
#include "lib/math/quaternion.h"

namespace Rayni
{
	namespace
	{
		testing::AssertionResult matrix_near(const char *m1_expr,
		                                     const char *m2_expr,
		                                     const char *abs_error_expr,
		                                     const Matrix3x3 &m1,
		                                     const Matrix3x3 &m2,
		                                     real_t abs_error)
		{
			std::string error_elements;

			auto error_elements_append = [&](auto row, auto column) {
				if (!error_elements.empty())
					error_elements += ", ";

				error_elements += "(" + std::to_string(row) + "," + std::to_string(column) + ")";
			};

			for (unsigned int row = 0; row < 3; row++)
			{
				for (unsigned int column = 0; column < 3; column++)
				{
					real_t diff = std::abs(m1(row, column) - m2(row, column));

					if (diff > abs_error)
						error_elements_append(row, column);
				}
			}

			if (error_elements.empty())
				return testing::AssertionSuccess();

			return testing::AssertionFailure() << "The matrix:\n"
			                                   << m1_expr << "\nand matrix:\n"
			                                   << m2_expr << "\ndiffer more than " << abs_error_expr
			                                   << " in the following elements:\n" + error_elements;
		}
	}

	TEST(Matrix3x3, OperatorIndexing)
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

	TEST(Matrix3x3, OperatorAddition)
	{
		Matrix3x3 m = Matrix3x3({1, 2, 3}, {4, 5, 6}, {7, 8, 9}) +
		              Matrix3x3({101, 102, 103}, {104, 105, 106}, {107, 108, 109});

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix3x3({102, 104, 106}, {108, 110, 112}, {114, 116, 118}),
		                    m,
		                    1e-100);
	}

	TEST(Matrix3x3, OperatorSubtraction)
	{
		Matrix3x3 m = Matrix3x3({101, 102, 103}, {104, 105, 106}, {107, 108, 109}) -
		              Matrix3x3({9, 8, 7}, {6, 5, 4}, {3, 2, 1});

		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({92, 94, 96}, {98, 100, 102}, {104, 106, 108}), m, 1e-100);
	}

	TEST(Matrix3x3, OperatorMultiplication)
	{
		Matrix3x3 m =
		        Matrix3x3({2, 16, 9}, {7, 8, 13}, {3, 15, 1}) * Matrix3x3({10, 3, 4}, {13, 8, 5}, {9, 10, 2});

		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({309, 224, 106}, {291, 215, 94}, {234, 139, 89}), m, 1e-100);
	}

	TEST(Matrix3x3, OperatorMultiplicationScalar)
	{
		Matrix3x3 m;

		m = Matrix3x3({1, 2, 3}, {4, 5, 6}, {7, 8, 9}) * 2;
		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({2, 4, 6}, {8, 10, 12}, {14, 16, 18}), m, 1e-100);

		m = 2 * Matrix3x3({1, 2, 3}, {4, 5, 6}, {7, 8, 9});
		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({2, 4, 6}, {8, 10, 12}, {14, 16, 18}), m, 1e-100);
	}

	TEST(Matrix3x3, SwapRows)
	{
		Matrix3x3 m({0, 1, 2}, {3, 4, 5}, {6, 7, 8});
		m.swap_rows(0, 2);

		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({6, 7, 8}, {3, 4, 5}, {0, 1, 2}), m, 1e-100);
	}

	TEST(Matrix3x3, SwapColumns)
	{
		Matrix3x3 m({0, 1, 2}, {3, 4, 5}, {6, 7, 8});
		m.swap_columns(0, 2);

		EXPECT_PRED_FORMAT3(matrix_near, Matrix3x3({2, 1, 0}, {5, 4, 3}, {8, 7, 6}), m, 1e-100);
	}

	TEST(Matrix3x3, Inverse)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix3x3({0.2647059, -0.1470588, 0.0588235},
		                              {-0.1470588, -0.0294118, 0.4117647},
		                              {0.0588235, 0.4117647, -0.7647059}),
		                    Matrix3x3({5, 3, 2}, {3, 7, 4}, {2, 4, 1}).inverse(),
		                    1e-7);
	}

	TEST(Matrix3x3, Transpose)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix3x3({0, 3, 6}, {1, 4, 7}, {2, 5, 8}),
		                    Matrix3x3({0, 1, 2}, {3, 4, 5}, {6, 7, 8}).transpose(),
		                    1e-100);
	}

	TEST(Matrix3x3, Trace)
	{
		EXPECT_NEAR(14, Matrix3x3({2, 100, 100}, {100, 4, 100}, {100, 100, 8}).trace(), 1e-100);
	}

	TEST(Matrix3x3, MaxDiagonalPosition)
	{
		EXPECT_EQ(0, Matrix3x3({3, 0, 0}, {0, 1, 0}, {0, 0, 2}).max_diagonal_position());
		EXPECT_EQ(1, Matrix3x3({2, 0, 0}, {0, 3, 0}, {0, 0, 1}).max_diagonal_position());
		EXPECT_EQ(2, Matrix3x3({1, 0, 0}, {0, 2, 0}, {0, 0, 3}).max_diagonal_position());
	}

	TEST(Matrix3x3, MaxAbsoluteRowSumNorm)
	{
		EXPECT_NEAR(60, Matrix3x3({10, 20, 30}, {1, 2, 3}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({-10, -20, -30}, {1, 2, 3}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);

		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {10, 20, 30}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {-10, -20, -30}, {4, 5, 6}).max_absolute_row_sum_norm(), 1e-100);

		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {4, 5, 6}, {10, 20, 30}).max_absolute_row_sum_norm(), 1e-100);
		EXPECT_NEAR(60, Matrix3x3({1, 2, 3}, {4, 5, 6}, {-10, -20, -30}).max_absolute_row_sum_norm(), 1e-100);
	}

	TEST(Matrix3x3, Rotation)
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

	TEST(Matrix3x3, PreservesOrientationOfBasis)
	{
		EXPECT_TRUE(Matrix3x3::scale(2).preserves_orientation_of_basis());
		EXPECT_FALSE(Matrix3x3::scale(-2).preserves_orientation_of_basis());
	}
}
