// This file is part of Rayni.
//
// Copyright (C) 2016-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/matrix4x4.h"

#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include "lib/math/math.h"

namespace Rayni
{
	namespace
	{
		testing::AssertionResult matrix_near(const char *m1_expr,
		                                     const char *m2_expr,
		                                     const char *abs_error_expr,
		                                     const Matrix4x4 &m1,
		                                     const Matrix4x4 &m2,
		                                     real_t abs_error)
		{
			std::string error_elements;

			auto error_elements_append = [&](auto row, auto column) {
				if (!error_elements.empty())
					error_elements += ", ";

				error_elements += "(" + std::to_string(row) + "," + std::to_string(column) + ")";
			};

			for (unsigned int row = 0; row < 4; row++)
			{
				for (unsigned int column = 0; column < 4; column++)
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

	TEST(Matrix4x4, OperatorIndexing)
	{
		Matrix4x4 m({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16});
		EXPECT_NEAR(1, m(0, 0), 1e-100);
		EXPECT_NEAR(2, m(0, 1), 1e-100);
		EXPECT_NEAR(3, m(0, 2), 1e-100);
		EXPECT_NEAR(4, m(0, 3), 1e-100);
		EXPECT_NEAR(5, m(1, 0), 1e-100);
		EXPECT_NEAR(6, m(1, 1), 1e-100);
		EXPECT_NEAR(7, m(1, 2), 1e-100);
		EXPECT_NEAR(8, m(1, 3), 1e-100);
		EXPECT_NEAR(9, m(2, 0), 1e-100);
		EXPECT_NEAR(10, m(2, 1), 1e-100);
		EXPECT_NEAR(11, m(2, 2), 1e-100);
		EXPECT_NEAR(12, m(2, 3), 1e-100);
		EXPECT_NEAR(13, m(3, 0), 1e-100);
		EXPECT_NEAR(14, m(3, 1), 1e-100);
		EXPECT_NEAR(15, m(3, 2), 1e-100);
		EXPECT_NEAR(16, m(3, 3), 1e-100);

		const Matrix4x4 mc({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16});
		EXPECT_NEAR(1, mc(0, 0), 1e-100);
		EXPECT_NEAR(2, mc(0, 1), 1e-100);
		EXPECT_NEAR(3, mc(0, 2), 1e-100);
		EXPECT_NEAR(4, mc(0, 3), 1e-100);
		EXPECT_NEAR(5, mc(1, 0), 1e-100);
		EXPECT_NEAR(6, mc(1, 1), 1e-100);
		EXPECT_NEAR(7, mc(1, 2), 1e-100);
		EXPECT_NEAR(8, mc(1, 3), 1e-100);
		EXPECT_NEAR(9, mc(2, 0), 1e-100);
		EXPECT_NEAR(10, mc(2, 1), 1e-100);
		EXPECT_NEAR(11, mc(2, 2), 1e-100);
		EXPECT_NEAR(12, mc(2, 3), 1e-100);
		EXPECT_NEAR(13, mc(3, 0), 1e-100);
		EXPECT_NEAR(14, mc(3, 1), 1e-100);
		EXPECT_NEAR(15, mc(3, 2), 1e-100);
		EXPECT_NEAR(16, mc(3, 3), 1e-100);
	}

	TEST(Matrix4x4, OperatorMultiplication)
	{
		Matrix4x4 m = Matrix4x4({2, 16, 9, 11}, {7, 8, 13, 14}, {3, 15, 1, 13}, {13, 1, 14, 12}) *
		              Matrix4x4({10, 3, 4, 6}, {13, 8, 5, 11}, {9, 10, 2, 16}, {1, 3, 2, 7});

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({320, 257, 128, 409},
		                              {305, 257, 122, 436},
		                              {247, 178, 115, 290},
		                              {281, 223, 109, 397}),
		                    m,
		                    1e-100);
	}

	TEST(Matrix4x4, SwapRows)
	{
		Matrix4x4 m({0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15});
		m.swap_rows(0, 3);

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({12, 13, 14, 15}, {4, 5, 6, 7}, {8, 9, 10, 11}, {0, 1, 2, 3}),
		                    m,
		                    1e-100);
	}

	TEST(Matrix4x4, SwapColumns)
	{
		Matrix4x4 m({0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15});
		m.swap_columns(0, 3);

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({3, 1, 2, 0}, {7, 5, 6, 4}, {11, 9, 10, 8}, {15, 13, 14, 12}),
		                    m,
		                    1e-100);
	}

	TEST(Matrix4x4, Inverse)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({0.2272727, -0.0454545, -0.4090909, 0.1363636},
		                              {0.0454545, -0.0757576, 0.3181818, -0.1060606},
		                              {-0.1727273, 0.4212121, -0.4090909, 0.0696970},
		                              {-0.1545455, -0.0090909, 0.3181818, 0.0272727}),
		                    Matrix4x4({5, 8, 2, 1}, {3, 7, 4, 2}, {2, 4, 1, 3}, {6, 1, 1, 8}).inverse(),
		                    1e-7);
	}

	TEST(Matrix4x4, Transpose)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({0, 4, 8, 12}, {1, 5, 9, 13}, {2, 6, 10, 14}, {3, 7, 11, 15}),
		                    Matrix4x4({0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}).transpose(),
		                    1e-100);
	}

	TEST(Matrix4x4, GetAxes)
	{
		auto m = Matrix4x4::from_axes({1, 2, 3}, {4, 5, 6}, {7, 8, 9});

		EXPECT_NEAR(1, m.x_axis().x(), 1e-100);
		EXPECT_NEAR(2, m.x_axis().y(), 1e-100);
		EXPECT_NEAR(3, m.x_axis().z(), 1e-100);

		EXPECT_NEAR(4, m.y_axis().x(), 1e-100);
		EXPECT_NEAR(5, m.y_axis().y(), 1e-100);
		EXPECT_NEAR(6, m.y_axis().z(), 1e-100);

		EXPECT_NEAR(7, m.z_axis().x(), 1e-100);
		EXPECT_NEAR(8, m.z_axis().y(), 1e-100);
		EXPECT_NEAR(9, m.z_axis().z(), 1e-100);
	}

	TEST(Matrix4x4, GetTranslation)
	{
		auto m = Matrix4x4::translate({2, 4, 8});

		EXPECT_NEAR(2, m.translation().x(), 1e-100);
		EXPECT_NEAR(4, m.translation().y(), 1e-100);
		EXPECT_NEAR(8, m.translation().z(), 1e-100);
	}

	TEST(Matrix4x4, GetRotation)
	{
		auto q = Matrix4x4::rotate({0.18257, 0.36515, 0.54772, 0.73030}).rotation();

		EXPECT_NEAR(0.18257, q.x(), 1e-5);
		EXPECT_NEAR(0.36515, q.y(), 1e-5);
		EXPECT_NEAR(0.54772, q.z(), 1e-5);
		EXPECT_NEAR(0.73030, q.w(), 1e-5);
	}

	TEST(Matrix4x4, Upper3x3)
	{
		Matrix3x3 m = Matrix4x4({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}).upper3x3();

		EXPECT_NEAR(1, m(0, 0), 1e-100);
		EXPECT_NEAR(2, m(0, 1), 1e-100);
		EXPECT_NEAR(3, m(0, 2), 1e-100);

		EXPECT_NEAR(5, m(1, 0), 1e-100);
		EXPECT_NEAR(6, m(1, 1), 1e-100);
		EXPECT_NEAR(7, m(1, 2), 1e-100);

		EXPECT_NEAR(9, m(2, 0), 1e-100);
		EXPECT_NEAR(10, m(2, 1), 1e-100);
		EXPECT_NEAR(11, m(2, 2), 1e-100);
	}
}
