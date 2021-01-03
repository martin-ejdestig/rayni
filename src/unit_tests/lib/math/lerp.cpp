// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/lerp.h"

#include <gtest/gtest.h>

#include <cmath>

#include "lib/math/math.h"

namespace Rayni
{
	namespace
	{
		struct Point
		{
			Point operator+(const Point &p) const
			{
				return {x + p.x, y + p.y};
			}

			Point operator-(const Point &p) const
			{
				return {x - p.x, y - p.y};
			}

			Point operator*(real_t s) const
			{
				return {x * s, y * s};
			}

			friend Point operator*(real_t s, const Point &p)
			{
				return {s * p.x, s * p.y};
			}

			float dot(const Point &p) const
			{
				return x * p.x + y * p.y;
			}

			Point normalize() const
			{
				real_t len_inv = 1 / std::sqrt(dot(*this));
				return *this * len_inv;
			}

			real_t x = 0;
			real_t y = 0;
		};
	}

	TEST(Lerp, Lerp)
	{
		EXPECT_EQ(5, lerp(-0.5, 10, 20));
		EXPECT_EQ(10, lerp(0.0, 10, 20));
		EXPECT_EQ(15, lerp(0.5, 10, 20));
		EXPECT_EQ(20, lerp(1.0, 10, 20));
		EXPECT_EQ(25, lerp(1.5, 10, 20));
	}

	TEST(Lerp, Blerp)
	{
		EXPECT_EQ(-5, blerp(-0.5, -0.5, 10, 20, 30, 40));
		EXPECT_EQ(0, blerp(0.0, -0.5, 10, 20, 30, 40));
		EXPECT_EQ(5, blerp(0.5, -0.5, 10, 20, 30, 40));
		EXPECT_EQ(10, blerp(1.0, -0.5, 10, 20, 30, 40));
		EXPECT_EQ(15, blerp(1.5, -0.5, 10, 20, 30, 40));

		EXPECT_EQ(5, blerp(-0.5, 0, 10, 20, 30, 40));
		EXPECT_EQ(10, blerp(0.0, 0, 10, 20, 30, 40));
		EXPECT_EQ(15, blerp(0.5, 0, 10, 20, 30, 40));
		EXPECT_EQ(20, blerp(1.0, 0, 10, 20, 30, 40));
		EXPECT_EQ(25, blerp(1.5, 0, 10, 20, 30, 40));

		EXPECT_EQ(15, blerp(-0.5, 0.5, 10, 20, 30, 40));
		EXPECT_EQ(20, blerp(0.0, 0.5, 10, 20, 30, 40));
		EXPECT_EQ(25, blerp(0.5, 0.5, 10, 20, 30, 40));
		EXPECT_EQ(30, blerp(1.0, 0.5, 10, 20, 30, 40));
		EXPECT_EQ(35, blerp(1.5, 0.5, 10, 20, 30, 40));

		EXPECT_EQ(25, blerp(-0.5, 1.0, 10, 20, 30, 40));
		EXPECT_EQ(30, blerp(0.0, 1.0, 10, 20, 30, 40));
		EXPECT_EQ(35, blerp(0.5, 1.0, 10, 20, 30, 40));
		EXPECT_EQ(40, blerp(1.0, 1.0, 10, 20, 30, 40));
		EXPECT_EQ(45, blerp(1.5, 1.0, 10, 20, 30, 40));

		EXPECT_EQ(35, blerp(-0.5, 1.5, 10, 20, 30, 40));
		EXPECT_EQ(40, blerp(0.0, 1.5, 10, 20, 30, 40));
		EXPECT_EQ(45, blerp(0.5, 1.5, 10, 20, 30, 40));
		EXPECT_EQ(50, blerp(1.0, 1.5, 10, 20, 30, 40));
		EXPECT_EQ(55, blerp(1.5, 1.5, 10, 20, 30, 40));
	}

	TEST(Lerp, Slerp)
	{
		static constexpr int CIRCLE_STEPS = 8;
		static constexpr int STEPS = 64;

		for (int start = 0; start < CIRCLE_STEPS; start++)
		{
			for (int end = start + 1; end < start + CIRCLE_STEPS / 2; end++)
			{
				real_t start_angle = 2 * PI * real_t(start) / CIRCLE_STEPS;
				real_t end_angle = 2 * PI * real_t(end) / CIRCLE_STEPS;
				Point start_point = {std::cos(start_angle), std::sin(start_angle)};
				Point end_point = {std::cos(end_angle), std::sin(end_angle)};

				for (int i = 0; i <= STEPS; i++)
				{
					Point p = slerp(real_t(i) / STEPS, start_point, end_point);
					real_t angle = start_angle + (end_angle - start_angle) * real_t(i) / STEPS;
					EXPECT_NEAR(std::cos(angle), p.x, 1e-6);
					EXPECT_NEAR(std::sin(angle), p.y, 1e-6);
				}
			}
		}
	}
}
