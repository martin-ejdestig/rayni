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

#include "lib/math/aabb.h"

#include <gtest/gtest.h>

#include "lib/math/ray.h"

namespace Rayni
{
	TEST(AABB, Merge)
	{
		AABB aabb;

		aabb.merge(AABB({0, 0, 0}, {1, 2, 3}))
		        .merge(AABB({-1, -2, -3}, {0, 0, 0}))
		        .merge(AABB({-0.5, -0.5, -0.5}, {0.5, 0.5, 0.5}));

		EXPECT_NEAR(-1, aabb.minimum().x(), 1e-100);
		EXPECT_NEAR(-2, aabb.minimum().y(), 1e-100);
		EXPECT_NEAR(-3, aabb.minimum().z(), 1e-100);
		EXPECT_NEAR(1, aabb.maximum().x(), 1e-100);
		EXPECT_NEAR(2, aabb.maximum().y(), 1e-100);
		EXPECT_NEAR(3, aabb.maximum().z(), 1e-100);
	}

	TEST(AABB, MergePoints)
	{
		AABB aabb;

		aabb.merge({1, 2, 3}).merge({-1, -2, -3}).merge({0.5, 0.5, 0.5}).merge({-0.5, -0.5, -0.5});

		EXPECT_NEAR(-1, aabb.minimum().x(), 1e-100);
		EXPECT_NEAR(-2, aabb.minimum().y(), 1e-100);
		EXPECT_NEAR(-3, aabb.minimum().z(), 1e-100);
		EXPECT_NEAR(1, aabb.maximum().x(), 1e-100);
		EXPECT_NEAR(2, aabb.maximum().y(), 1e-100);
		EXPECT_NEAR(3, aabb.maximum().z(), 1e-100);
	}

	TEST(AABB, IntersectsHit)
	{
		AABB aabb({-1, -1, -1}, {1, 1, 1});

		auto hit_rays = {Ray({5, 0, 0}, {-1, 0, 0}, 0),
		                 Ray({-5, 0, 0}, {1, 0, 0}, 0),
		                 Ray({0, 5, 0}, {0, -1, 0}, 0),
		                 Ray({0, -5, 0}, {0, 1, 0}, 0),
		                 Ray({0, 0, 5}, {0, 0, -1}, 0),
		                 Ray({0, 0, -5}, {0, 0, 1}, 0)};

		for (const Ray &ray : hit_rays)
		{
			real_t t_min = 0, t_max = 0;
			EXPECT_TRUE(aabb.intersects(ray, t_min, t_max));
			EXPECT_NEAR(4, t_min, 1e-100);
			EXPECT_NEAR(6, t_max, 1e-100);
		}
	}

	TEST(AABB, IntersectsInside)
	{
		AABB aabb({-1, -1, -1}, {1, 1, 1});

		auto inside_rays = {Ray({-0.5, 0, 0}, {-1, 0, 0}, 0),
		                    Ray({0.5, 0, 0}, {1, 0, 0}, 0),
		                    Ray({0, -0.5, 0}, {0, -1, 0}, 0),
		                    Ray({0, 0.5, 0}, {0, 1, 0}, 0),
		                    Ray({0, 0, -0.5}, {0, 0, -1}, 0),
		                    Ray({0, 0, 0.5}, {0, 0, 1}, 0)};

		for (const Ray &ray : inside_rays)
		{
			real_t t_min = 1, t_max = 0;
			EXPECT_TRUE(aabb.intersects(ray, t_min, t_max));
			EXPECT_NEAR(0, t_min, 1e-100);
			EXPECT_NEAR(0.5, t_max, 1e-100);
		}
	}

	TEST(AABB, IntersectsMiss)
	{
		AABB aabb({-1, -1, -1}, {1, 1, 1});

		auto miss_rays =
		        {Ray({5, 0, 0}, {1, 0, 0}, 0),   Ray({5, 0, 0}, {0, 1, 0}, 0),   Ray({5, 0, 0}, {0, -1, 0}, 0),
		         Ray({5, 0, 0}, {0, 0, 1}, 0),   Ray({5, 0, 0}, {0, 0, -1}, 0),

		         Ray({-5, 0, 0}, {-1, 0, 0}, 0), Ray({-5, 0, 0}, {0, 1, 0}, 0),  Ray({-5, 0, 0}, {0, -1, 0}, 0),
		         Ray({-5, 0, 0}, {0, 0, 1}, 0),  Ray({-5, 0, 0}, {0, 0, -1}, 0),

		         Ray({0, 5, 0}, {1, 0, 0}, 0),   Ray({0, 5, 0}, {-1, 0, 0}, 0),  Ray({0, 5, 0}, {0, 1, 0}, 0),
		         Ray({0, 5, 0}, {0, 0, 1}, 0),   Ray({0, 5, 0}, {0, 0, -1}, 0),

		         Ray({0, -5, 0}, {1, 0, 0}, 0),  Ray({0, -5, 0}, {-1, 0, 0}, 0), Ray({0, -5, 0}, {0, -1, 0}, 0),
		         Ray({0, -5, 0}, {0, 0, 1}, 0),  Ray({0, -5, 0}, {0, 0, -1}, 0),

		         Ray({0, 0, 5}, {1, 0, 0}, 0),   Ray({0, 0, 5}, {-1, 0, 0}, 0),  Ray({0, 0, 5}, {0, 1, 0}, 0),
		         Ray({0, 0, 5}, {0, -1, 0}, 0),  Ray({0, 0, 5}, {0, 0, 1}, 0),

		         Ray({0, 0, -5}, {1, 0, 0}, 0),  Ray({0, 0, -5}, {-1, 0, 0}, 0), Ray({0, 0, -5}, {0, 1, 0}, 0),
		         Ray({0, 0, -5}, {0, -1, 0}, 0), Ray({0, 0, -5}, {0, 0, -1}, 0)};

		for (const Ray &ray : miss_rays)
		{
			real_t t_min = 12345, t_max = 67890;
			EXPECT_FALSE(aabb.intersects(ray, t_min, t_max));
			EXPECT_NEAR(12345, t_min, 1e-100);
			EXPECT_NEAR(67890, t_max, 1e-100);
		}
	}

	TEST(AABB, Intersection)
	{
		AABB intersection =
		        AABB({-10, -100, -1000}, {5, 50, 500}).intersection(AABB({-5, -50, -500}, {10, 100, 1000}));

		EXPECT_NEAR(-5, intersection.minimum().x(), 1e-100);
		EXPECT_NEAR(-50, intersection.minimum().y(), 1e-100);
		EXPECT_NEAR(-500, intersection.minimum().z(), 1e-100);
		EXPECT_NEAR(5, intersection.maximum().x(), 1e-100);
		EXPECT_NEAR(50, intersection.maximum().y(), 1e-100);
		EXPECT_NEAR(500, intersection.maximum().z(), 1e-100);
	}

	TEST(AABB, SurfaceArea)
	{
		EXPECT_NEAR(286, AABB({-1, -2, -3}, {4, 5, 6}).surface_area(), 1e-100);
	}

	TEST(AABB, SplitX)
	{
		auto split_x = AABB({10, 100, 1000}, {20, 200, 2000}).split(0, 15);

		EXPECT_NEAR(10, split_x.left.minimum().x(), 1e-100);
		EXPECT_NEAR(100, split_x.left.minimum().y(), 1e-100);
		EXPECT_NEAR(1000, split_x.left.minimum().z(), 1e-100);
		EXPECT_NEAR(15, split_x.left.maximum().x(), 1e-100);
		EXPECT_NEAR(200, split_x.left.maximum().y(), 1e-100);
		EXPECT_NEAR(2000, split_x.left.maximum().z(), 1e-100);
		EXPECT_NEAR(15, split_x.right.minimum().x(), 1e-100);
		EXPECT_NEAR(100, split_x.right.minimum().y(), 1e-100);
		EXPECT_NEAR(1000, split_x.right.minimum().z(), 1e-100);
		EXPECT_NEAR(20, split_x.right.maximum().x(), 1e-100);
		EXPECT_NEAR(200, split_x.right.maximum().y(), 1e-100);
		EXPECT_NEAR(2000, split_x.right.maximum().z(), 1e-100);
	}

	TEST(AABB, SplitY)
	{
		auto split_y = AABB({10, 100, 1000}, {20, 200, 2000}).split(1, 150);

		EXPECT_NEAR(10, split_y.left.minimum().x(), 1e-100);
		EXPECT_NEAR(100, split_y.left.minimum().y(), 1e-100);
		EXPECT_NEAR(1000, split_y.left.minimum().z(), 1e-100);
		EXPECT_NEAR(20, split_y.left.maximum().x(), 1e-100);
		EXPECT_NEAR(150, split_y.left.maximum().y(), 1e-100);
		EXPECT_NEAR(2000, split_y.left.maximum().z(), 1e-100);
		EXPECT_NEAR(10, split_y.right.minimum().x(), 1e-100);
		EXPECT_NEAR(150, split_y.right.minimum().y(), 1e-100);
		EXPECT_NEAR(1000, split_y.right.minimum().z(), 1e-100);
		EXPECT_NEAR(20, split_y.right.maximum().x(), 1e-100);
		EXPECT_NEAR(200, split_y.right.maximum().y(), 1e-100);
		EXPECT_NEAR(2000, split_y.right.maximum().z(), 1e-100);
	}

	TEST(AABB, SplitZ)
	{
		auto split_z = AABB({10, 100, 1000}, {20, 200, 2000}).split(2, 1500);

		EXPECT_NEAR(10, split_z.left.minimum().x(), 1e-100);
		EXPECT_NEAR(100, split_z.left.minimum().y(), 1e-100);
		EXPECT_NEAR(1000, split_z.left.minimum().z(), 1e-100);
		EXPECT_NEAR(20, split_z.left.maximum().x(), 1e-100);
		EXPECT_NEAR(200, split_z.left.maximum().y(), 1e-100);
		EXPECT_NEAR(1500, split_z.left.maximum().z(), 1e-100);
		EXPECT_NEAR(10, split_z.right.minimum().x(), 1e-100);
		EXPECT_NEAR(100, split_z.right.minimum().y(), 1e-100);
		EXPECT_NEAR(1500, split_z.right.minimum().z(), 1e-100);
		EXPECT_NEAR(20, split_z.right.maximum().x(), 1e-100);
		EXPECT_NEAR(200, split_z.right.maximum().y(), 1e-100);
		EXPECT_NEAR(2000, split_z.right.maximum().z(), 1e-100);
	}

	TEST(AABB, IsPlanar)
	{
		EXPECT_FALSE(AABB({1, 1, 1}, {2, 2, 2}).is_planar(0));
		EXPECT_FALSE(AABB({1, 1, 1}, {2, 2, 2}).is_planar(1));
		EXPECT_FALSE(AABB({1, 1, 1}, {2, 2, 2}).is_planar(2));

		EXPECT_TRUE(AABB({1, 1, 1}, {1, 2, 2}).is_planar(0));
		EXPECT_FALSE(AABB({1, 1, 1}, {1, 2, 2}).is_planar(1));
		EXPECT_FALSE(AABB({1, 1, 1}, {1, 2, 2}).is_planar(2));

		EXPECT_FALSE(AABB({1, 1, 1}, {2, 1, 2}).is_planar(0));
		EXPECT_TRUE(AABB({1, 1, 1}, {2, 1, 2}).is_planar(1));
		EXPECT_FALSE(AABB({1, 1, 1}, {2, 1, 2}).is_planar(2));

		EXPECT_FALSE(AABB({1, 1, 1}, {2, 2, 1}).is_planar(0));
		EXPECT_FALSE(AABB({1, 1, 1}, {2, 2, 1}).is_planar(1));
		EXPECT_TRUE(AABB({1, 1, 1}, {2, 2, 1}).is_planar(2));
	}
}
