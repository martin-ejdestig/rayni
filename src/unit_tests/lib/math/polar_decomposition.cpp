/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/polar_decomposition.h"

#include <gtest/gtest.h>

#include "lib/math/matrix3x3.h"

namespace Rayni
{
	TEST(PolarDecomposition, Calculate)
	{
		PolarDecomposition<Matrix3x3> pd(Matrix3x3({0.936293, -0.579258, 0.596007},
		                                           {0.312992, 1.88941, -0.29353},
		                                           {-0.159345, 0.307584, 2.92551}));

		EXPECT_NEAR(0.936293, pd.rotation(0, 0), 1e-4);
		EXPECT_NEAR(-0.289629, pd.rotation(0, 1), 1e-4);
		EXPECT_NEAR(0.198669, pd.rotation(0, 2), 1e-4);
		EXPECT_NEAR(0.312992, pd.rotation(1, 0), 1e-4);
		EXPECT_NEAR(0.944703, pd.rotation(1, 1), 1e-4);
		EXPECT_NEAR(-0.0978434, pd.rotation(1, 2), 1e-4);
		EXPECT_NEAR(-0.159345, pd.rotation(2, 0), 1e-4);
		EXPECT_NEAR(0.153792, pd.rotation(2, 1), 1e-4);
		EXPECT_NEAR(0.97517, pd.rotation(2, 2), 1e-4);

		EXPECT_NEAR(1, pd.scale(0, 0), 1e-4);
		EXPECT_NEAR(0, pd.scale(0, 1), 1e-4);
		EXPECT_NEAR(0, pd.scale(0, 2), 1e-4);
		EXPECT_NEAR(0, pd.scale(1, 0), 1e-4);
		EXPECT_NEAR(2, pd.scale(1, 1), 1e-4);
		EXPECT_NEAR(0, pd.scale(1, 2), 1e-4);
		EXPECT_NEAR(0, pd.scale(2, 0), 1e-4);
		EXPECT_NEAR(0, pd.scale(2, 1), 1e-4);
		EXPECT_NEAR(3, pd.scale(2, 2), 1e-4);
	}
}
