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
#include "lib/math/matrix4x4.h"

namespace Rayni
{
	class Matrix4x4Test : public testing::Test
	{
	protected:
		static testing::AssertionResult matrix_near(const char *m1_expr,
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
	};

	TEST_F(Matrix4x4Test, Inverse)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({0.2272727, -0.0454545, -0.4090909, 0.1363636},
		                              {0.0454545, -0.0757576, 0.3181818, -0.1060606},
		                              {-0.1727273, 0.4212121, -0.4090909, 0.0696970},
		                              {-0.1545455, -0.0090909, 0.3181818, 0.0272727}),
		                    Matrix4x4({5, 8, 2, 1}, {3, 7, 4, 2}, {2, 4, 1, 3}, {6, 1, 1, 8}).inverse(),
		                    1e-7);
	}

	TEST_F(Matrix4x4Test, Transpose)
	{
		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({0, 4, 8, 12}, {1, 5, 9, 13}, {2, 6, 10, 14}, {3, 7, 11, 15}),
		                    Matrix4x4({0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}).transpose(),
		                    1e-100);
	}

	TEST_F(Matrix4x4Test, GetAxes)
	{
		auto m = Matrix4x4::from_axes({1, 2, 3}, {4, 5, 6}, {7, 8, 9});

		EXPECT_NEAR(1, m.get_x_axis().x(), 1e-100);
		EXPECT_NEAR(2, m.get_x_axis().y(), 1e-100);
		EXPECT_NEAR(3, m.get_x_axis().z(), 1e-100);

		EXPECT_NEAR(4, m.get_y_axis().x(), 1e-100);
		EXPECT_NEAR(5, m.get_y_axis().y(), 1e-100);
		EXPECT_NEAR(6, m.get_y_axis().z(), 1e-100);

		EXPECT_NEAR(7, m.get_z_axis().x(), 1e-100);
		EXPECT_NEAR(8, m.get_z_axis().y(), 1e-100);
		EXPECT_NEAR(9, m.get_z_axis().z(), 1e-100);
	}

	TEST_F(Matrix4x4Test, GetTranslation)
	{
		auto m = Matrix4x4::translate({2, 4, 8});

		EXPECT_NEAR(2, m.get_translation().x(), 1e-100);
		EXPECT_NEAR(4, m.get_translation().y(), 1e-100);
		EXPECT_NEAR(8, m.get_translation().z(), 1e-100);
	}

	TEST_F(Matrix4x4Test, GetRotation)
	{
		auto q = Matrix4x4::rotate({0.18257, 0.36515, 0.54772, 0.73030}).get_rotation();

		EXPECT_NEAR(0.18257, q.x(), 1e-5);
		EXPECT_NEAR(0.36515, q.y(), 1e-5);
		EXPECT_NEAR(0.54772, q.z(), 1e-5);
		EXPECT_NEAR(0.73030, q.w(), 1e-5);
	}

	TEST_F(Matrix4x4Test, Upper3x3Trace)
	{
		EXPECT_NEAR(14,
		            Matrix4x4({2, 100, 100, 100}, {100, 4, 100, 100}, {100, 100, 8, 100}, {100, 100, 100, 100})
		                    .upper3x3_trace(),
		            1e-100);
	}

	TEST_F(Matrix4x4Test, Upper3x3MaxDiagonalPosition)
	{
		EXPECT_EQ(0,
		          Matrix4x4({3, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 2, 0}, {0, 0, 0, 4})
		                  .upper3x3_max_diagonal_position());
		EXPECT_EQ(1,
		          Matrix4x4({2, 0, 0, 0}, {0, 3, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 4})
		                  .upper3x3_max_diagonal_position());
		EXPECT_EQ(2,
		          Matrix4x4({1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {0, 0, 0, 4})
		                  .upper3x3_max_diagonal_position());
	}

	TEST_F(Matrix4x4Test, PolarDecomposition)
	{
		Matrix4x4 m({0.936293, -0.579258, 0.596007, 0},
		            {0.312992, 1.88941, -0.29353, 0},
		            {-0.159345, 0.307584, 2.92551, 0},
		            {0, 0, 0, 1});

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({0.936293, -0.289629, 0.198669, 0},
		                              {0.312992, 0.944703, -0.0978434, 0},
		                              {-0.159345, 0.153792, 0.97517, 0},
		                              {0, 0, 0, 1}),
		                    m.polar_decomposition().rotation,
		                    1e-4);

		EXPECT_PRED_FORMAT3(matrix_near,
		                    Matrix4x4({1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {0, 0, 0, 1}),
		                    m.polar_decomposition().scale,
		                    1e-4);
	}

	TEST_F(Matrix4x4Test, PreservesOrientationOfBasis)
	{
		EXPECT_TRUE(Matrix4x4::scale(2).preserves_orientation_of_basis());
		EXPECT_FALSE(Matrix4x4::scale(-2).preserves_orientation_of_basis());
	}
}
