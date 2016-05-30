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

#include "lib/containers/variant.h"
#include "lib/math/animated_transform.h"

namespace Rayni
{
	class AnimatedTransformTest : public testing::Test
	{
	protected:
		static testing::AssertionResult animated_transform_interpolate_near(
		        const char *animated_transform_expr,
		        const char *transform_expr,
		        const char * /*time_expr*/,
		        const char *abs_error_expr,
		        const AnimatedTransform &animated_transform,
		        const Transform &transform,
		        real_t time,
		        real_t abs_error)
		{
			auto transform_at_time = animated_transform.interpolate(time);
			auto m1 = transform_at_time.get_matrix();
			auto m1_inv = transform_at_time.inverse().get_matrix();
			auto m2 = transform.get_matrix();
			auto m2_inv = transform.inverse().get_matrix();

			for (unsigned int i = 0; i < 4; i++)
			{
				for (unsigned int j = 0; j < 4; j++)
					if (std::abs(m1(i, j) - m2(i, j)) > abs_error ||
					    std::abs(m1_inv(i, j) - m2_inv(i, j)) > abs_error)
						return testing::AssertionFailure()
						       << animated_transform_expr << " has elements that differ more "
						       << "than " << abs_error_expr << " from " << transform_expr
						       << " at time " << time << ".";
			}

			return testing::AssertionSuccess();
		}
	};

	TEST_F(AnimatedTransformTest, Variant)
	{
		auto double_scale_variant = [](real_t start_time, real_t end_time) {
			return Variant::map("start_time",
			                    start_time,
			                    "start_transform",
			                    "identity",
			                    "end_time",
			                    end_time,
			                    "end_transform",
			                    Variant::map("scale", 2));
		};

		EXPECT_PRED_FORMAT4(animated_transform_interpolate_near,
		                    AnimatedTransform(double_scale_variant(0, 2)),
		                    Transform::scale(1.5),
		                    1,
		                    1e-7);

		EXPECT_THROW(AnimatedTransform(double_scale_variant(2, 0)), Variant::Exception);
	}

	TEST_F(AnimatedTransformTest, Interpolate)
	{
		const unsigned int START_TIME = 0;
		const unsigned int ANIM_START_TIME = START_TIME + 1;
		const unsigned int ANIM_END_TIME = ANIM_START_TIME + 4;
		const unsigned int END_TIME = ANIM_END_TIME + 1;

		auto create_transform = [&](unsigned int time) {
			real_t ratio;

			if (time <= ANIM_START_TIME)
				ratio = real_t(0);
			else if (time >= ANIM_END_TIME)
				ratio = real_t(1);
			else
				ratio = real_t(time - ANIM_START_TIME) / (ANIM_END_TIME - ANIM_START_TIME);

			return Transform::combine(Transform::translate(Vector3(-1, -1, -1) + Vector3(2, 2, 2) * ratio),
			                          Transform::rotate_z(PI * real_t(0.5) * ratio));
		};

		const auto start_transform = create_transform(ANIM_START_TIME);
		const auto end_transform = create_transform(ANIM_END_TIME);

		const AnimatedTransform animated_transform(real_t(ANIM_START_TIME),
		                                           start_transform,
		                                           real_t(ANIM_END_TIME),
		                                           end_transform);

		for (unsigned int time = START_TIME; time <= ANIM_START_TIME; time++)
			EXPECT_PRED_FORMAT4(animated_transform_interpolate_near,
			                    animated_transform,
			                    start_transform,
			                    real_t(time),
			                    1e-6);

		for (unsigned int time = ANIM_START_TIME + 1; time <= ANIM_END_TIME - 1; time++)
			EXPECT_PRED_FORMAT4(animated_transform_interpolate_near,
			                    animated_transform,
			                    create_transform(time),
			                    real_t(time),
			                    1e-6);

		for (unsigned int time = ANIM_END_TIME; time <= END_TIME; time++)
			EXPECT_PRED_FORMAT4(animated_transform_interpolate_near,
			                    animated_transform,
			                    end_transform,
			                    real_t(time),
			                    1e-6);
	}

	TEST_F(AnimatedTransformTest, MotionBounds)
	{
		AABB aabb = AnimatedTransform(0, Transform::identity(), 1, Transform::scale(2))
		                    .motion_bounds(AABB({-1, -2, -3}, {4, 5, 6}));

		EXPECT_NEAR(-2, aabb.get_minimum().x(), 1e-6);
		EXPECT_NEAR(-4, aabb.get_minimum().y(), 1e-6);
		EXPECT_NEAR(-6, aabb.get_minimum().z(), 1e-6);
		EXPECT_NEAR(8, aabb.get_maximum().x(), 1e-6);
		EXPECT_NEAR(10, aabb.get_maximum().y(), 1e-6);
		EXPECT_NEAR(12, aabb.get_maximum().z(), 1e-6);
	}
}
