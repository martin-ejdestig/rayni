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

#include "lib/math/decomposed_matrix4x4.h"

#include <gtest/gtest.h>

#include <cmath>

#include "lib/math/matrix4x4.h"

namespace Rayni
{
	TEST(DecomposedMatrix4x4, Interpolate)
	{
		Matrix4x4 start({2, 0, 0, 2},
		                {0, std::cos(real_t(1)) * 2, -std::sin(real_t(1)) * 2, 3},
		                {0, std::sin(real_t(1)) * 2, std::cos(real_t(1)) * 2, 4},
		                {0, 0, 0, 1});

		Matrix4x4 end({4, 0, 0, 4},
		              {0, std::cos(real_t(3)) * 4, -std::sin(real_t(3)) * 4, 5},
		              {0, std::sin(real_t(3)) * 4, std::cos(real_t(3)) * 4, 6},
		              {0, 0, 0, 1});

		Matrix4x4 middle = DecomposedMatrix4x4(start).interpolate(0.5, DecomposedMatrix4x4(end)).compose();

		EXPECT_NEAR(3, middle(0, 0), 1e-6);
		EXPECT_NEAR(0, middle(0, 1), 1e-6);
		EXPECT_NEAR(0, middle(0, 2), 1e-6);
		EXPECT_NEAR(3, middle(0, 3), 1e-6);

		EXPECT_NEAR(0, middle(1, 0), 1e-6);
		EXPECT_NEAR(std::cos(real_t(2)) * 3, middle(1, 1), 1e-6);
		EXPECT_NEAR(-std::sin(real_t(2)) * 3, middle(1, 2), 1e-6);
		EXPECT_NEAR(4, middle(1, 3), 1e-6);

		EXPECT_NEAR(0, middle(2, 0), 1e-6);
		EXPECT_NEAR(std::sin(real_t(2)) * 3, middle(2, 1), 1e-6);
		EXPECT_NEAR(std::cos(real_t(2)) * 3, middle(2, 2), 1e-6);
		EXPECT_NEAR(5, middle(2, 3), 1e-6);

		EXPECT_NEAR(0, middle(3, 0), 1e-6);
		EXPECT_NEAR(0, middle(3, 1), 1e-6);
		EXPECT_NEAR(0, middle(3, 2), 1e-6);
		EXPECT_NEAR(1, middle(3, 3), 1e-6);
	}
}
