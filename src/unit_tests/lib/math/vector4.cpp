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

#include "lib/math/vector4.h"

namespace Rayni
{
	TEST(Vector4Test, Operators)
	{
		Vector4 v;

		v = Vector4(1, 2, 3, 4);
		EXPECT_EQ(v.x(), v[0]);
		EXPECT_EQ(v.y(), v[1]);
		EXPECT_EQ(v.z(), v[2]);
		EXPECT_EQ(v.w(), v[3]);

		const Vector4 vc(1, 2, 3, 4);
		EXPECT_EQ(vc.x(), v[0]);
		EXPECT_EQ(vc.y(), v[1]);
		EXPECT_EQ(vc.z(), v[2]);
		EXPECT_EQ(vc.w(), v[3]);

		v = Vector4(1, 2, 3, 4) + Vector4(5, 6, 7, 8);
		EXPECT_NEAR(6, v.x(), 1e-100);
		EXPECT_NEAR(8, v.y(), 1e-100);
		EXPECT_NEAR(10, v.z(), 1e-100);
		EXPECT_NEAR(12, v.w(), 1e-100);

		v = Vector4(8, 7, 6, 5) - Vector4(1, 2, 3, 4);
		EXPECT_NEAR(7, v.x(), 1e-100);
		EXPECT_NEAR(5, v.y(), 1e-100);
		EXPECT_NEAR(3, v.z(), 1e-100);
		EXPECT_NEAR(1, v.w(), 1e-100);

		v = Vector4(1, 2, 3, 4) * real_t(2);
		EXPECT_NEAR(2, v.x(), 1e-100);
		EXPECT_NEAR(4, v.y(), 1e-100);
		EXPECT_NEAR(6, v.z(), 1e-100);
		EXPECT_NEAR(8, v.w(), 1e-100);

		v = Vector4(1, 2, 3, 4);
		v += Vector4(5, 6, 7, 8);
		EXPECT_NEAR(6, v.x(), 1e-100);
		EXPECT_NEAR(8, v.y(), 1e-100);
		EXPECT_NEAR(10, v.z(), 1e-100);
		EXPECT_NEAR(12, v.w(), 1e-100);

		v = Vector4(1, 2, 3, 4);
		v *= real_t(2);
		EXPECT_NEAR(2, v.x(), 1e-100);
		EXPECT_NEAR(4, v.y(), 1e-100);
		EXPECT_NEAR(6, v.z(), 1e-100);
		EXPECT_NEAR(8, v.w(), 1e-100);
	}

	TEST(Vector4Test, Dot)
	{
		EXPECT_NEAR(7000, Vector4(10, 20, 30, 40).dot(Vector4(50, 60, 70, 80)), 1e-100);
		EXPECT_NEAR(0, Vector4(0, 0, 0, 0).dot(Vector4(50, 60, 70, 80)), 1e-100);
		EXPECT_NEAR(0, Vector4(10, 20, 30, 40).dot(Vector4(0, 0, 0, 0)), 1e-100);
	}
}
