/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/shapes/triangle_mesh_data.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include "lib/math/vector3.h"

namespace Rayni
{
	namespace
	{
		testing::AssertionResult normals_near(const char *normals1_expr,
		                                      const char *normals2_expr,
		                                      const char *abs_error_expr,
		                                      const std::vector<Vector3> &normals1,
		                                      const std::vector<Vector3> &normals2,
		                                      real_t abs_error)
		{
			if (normals1.size() != normals2.size())
				return testing::AssertionFailure()
				       << "Number of normals differ:\n"
				       << "size of " << normals1_expr << " is " << normals1.size() << "\n"
				       << "size of " << normals2_expr << " is " << normals2.size();

			for (std::size_t i = 0; i < normals1.size(); i++)
			{
				Vector3 diff = normals1[i] - normals2[i];

				for (unsigned int j = 0; j < 3; j++)
				{
					if (std::abs(diff[j]) > abs_error)
						return testing::AssertionFailure()
						       << "Normal component " << j << " of normal " << i
						       << " differs more than " << abs_error_expr << ".\n"
						       << "(" << normals1[i][0] << ", " << normals1[i][1] << ", "
						       << normals1[i][2] << ") from " << normals1_expr << "\n"
						       << "(" << normals2[i][0] << ", " << normals2[i][1] << ", "
						       << normals2[i][2] << ") from " << normals2_expr;
				}
			}

			return testing::AssertionSuccess();
		}
	}

	TEST(TriangleMeshData, CalculateNormalsFacingCorrectlyForRightHandSystem)
	{
		TriangleMeshData data;
		std::vector<Vector3> expected_normals;

		for (unsigned int normal_axis = 0; normal_axis < 3; normal_axis++)
		{
			TriangleMeshData::Index index = normal_axis * 6;
			unsigned int axis1 = (normal_axis + 1) % 3;
			unsigned int axis2 = (normal_axis + 2) % 3;
			Vector3 n, p1, p2, p3;

			n[normal_axis] = 1;
			p1[axis1] = 1;
			p2[axis2] = 1;
			p3[axis1] = -1;

			data.points.emplace_back(p1); // Counter clockwise.
			data.points.emplace_back(p2);
			data.points.emplace_back(p3);
			expected_normals.emplace_back(n);
			expected_normals.emplace_back(n);
			expected_normals.emplace_back(n);
			data.indices.emplace_back(index + 0, index + 1, index + 2);

			data.points.emplace_back(p1); // Clockwise, flipped normal.
			data.points.emplace_back(p3);
			data.points.emplace_back(p2);
			expected_normals.emplace_back(-n);
			expected_normals.emplace_back(-n);
			expected_normals.emplace_back(-n);
			data.indices.emplace_back(index + 3, index + 4, index + 5);
		}

		TriangleMeshData::calculate_normals(data);

		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);
	}

	TEST(TriangleMeshData, CalculateNormalsContributionRelativeToTriangleArea)
	{
		TriangleMeshData data;
		data.points = {{0, 0, 0}, {0, 0, 1}, {0, 0, -1}, {0, 0, -1}, {0, 0, 1}};
		data.indices = {{0, 1, 2}, {0, 3, 4}};

		std::vector<Vector3> expected_normals = {{0, 0, 0}, {0, 1, 0}, {0, 1, 0}, {1, 0, 0}, {1, 0, 0}};

		// Area of triangle in xz plane same as area of triangle in yz plane, same contribution.
		data.points[1][0] = 1;
		data.points[2][0] = 1;
		data.points[3][1] = 1;
		data.points[4][1] = 1;
		expected_normals[0] = Vector3(1, 1, 0).normalize();
		TriangleMeshData::calculate_normals(data);
		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);

		// Area of triangle in xz plane * 2, y normal contributes * 2.
		data.points[1][0] = 2;
		data.points[2][0] = 2;
		data.points[3][1] = 1;
		data.points[4][1] = 1;
		expected_normals[0] = Vector3(1, 2, 0).normalize();
		TriangleMeshData::calculate_normals(data);
		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);

		// Area of triangle in xz plane * 3, y normal contributes * 3.
		data.points[1][0] = 3;
		data.points[2][0] = 3;
		data.points[3][1] = 1;
		data.points[4][1] = 1;
		expected_normals[0] = Vector3(1, 3, 0).normalize();
		TriangleMeshData::calculate_normals(data);
		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);

		// Area of triangle in yz plane * 2, x normal contributes * 2.
		data.points[1][0] = 1;
		data.points[2][0] = 1;
		data.points[3][1] = 2;
		data.points[4][1] = 2;
		expected_normals[0] = Vector3(2, 1, 0).normalize();
		TriangleMeshData::calculate_normals(data);
		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);

		// Area of triangle in yz plane * 3, x normal contributes * 3.
		data.points[1][0] = 1;
		data.points[2][0] = 1;
		data.points[3][1] = 3;
		data.points[4][1] = 3;
		expected_normals[0] = Vector3(3, 1, 0).normalize();
		TriangleMeshData::calculate_normals(data);
		EXPECT_PRED_FORMAT3(normals_near, data.normals, expected_normals, 1e-7);
	}
}
