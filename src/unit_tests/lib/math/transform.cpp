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

#include "lib/math/transform.h"

#include <gtest/gtest.h>

#include <cmath>

#include "lib/containers/variant.h"
#include "lib/math/aabb.h"
#include "lib/math/math.h"
#include "lib/math/matrix4x4.h"
#include "lib/math/ray.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	namespace
	{
		testing::AssertionResult transform_near(const Transform &t1, const Transform &t2, real_t abs_error)
		{
			for (unsigned int i = 0; i < 4; i++)
				for (unsigned int j = 0; j < 4; j++)
					if (std::abs(t1.matrix()(i, j) - t2.matrix()(i, j)) > abs_error)
						return testing::AssertionFailure();

			return testing::AssertionSuccess();
		}

		testing::AssertionResult transform_near(const char *t1_expr,
		                                        const char *t2_expr,
		                                        const char *abs_error_expr,
		                                        const Transform &t1,
		                                        const Transform &t2,
		                                        real_t abs_error)
		{
			return transform_near(t1, t2, abs_error)
			       << t1_expr << " has elements that differ more than " << abs_error_expr
			       << " from elements of " << t2_expr << ".";
		}

		testing::AssertionResult verify_inverse(const char *expr,
		                                        const char *abs_error_expr,
		                                        const Transform &t,
		                                        real_t abs_error)
		{
			return transform_near(t.inverse(), Transform(t.matrix().inverse(), t.matrix()), abs_error)
			       << "Inverse of " << expr << ", calculated directly from input values if possible, has "
			       << "elements that differ more than " << abs_error_expr << " from elements of inverse "
			       << "calculated from transformation matrix.";
		}
	}

	TEST(Transform, VariantString)
	{
		EXPECT_PRED_FORMAT3(transform_near, Variant("identity").to<Transform>(), Transform::identity(), 1e-100);

		EXPECT_THROW(Variant("foo").to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantMapSizeLargerThanOneNotAllowed)
	{
		EXPECT_THROW(Variant::map("rotate_x", 1, "rotate_y", 2).to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantMapTranslate)
	{
		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("translate", Variant::vector(1, 2, 3)).to<Transform>(),
		                    Transform::translate({1, 2, 3}),
		                    1e-100);

		EXPECT_THROW(Variant::map("translate", Variant::vector(1, 2)).to<Transform>(), Variant::Exception);
		EXPECT_THROW(Variant::map("translate", 1).to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantMapScale)
	{
		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("scale", Variant::vector(1, 2, 3)).to<Transform>(),
		                    Transform::scale(1, 2, 3),
		                    1e-100);

		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("scale", 2).to<Transform>(),
		                    Transform::scale(2),
		                    1e-100);

		EXPECT_THROW(Variant::map("scale", Variant::vector(1, 2)).to<Transform>(), Variant::Exception);
		EXPECT_THROW(Variant::map("scale", "foo").to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantMapRotate)
	{
		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("rotate_x", 30).to<Transform>(),
		                    Transform::rotate_x(radians_from_degrees(30)),
		                    1e-7);

		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("rotate_y", 30).to<Transform>(),
		                    Transform::rotate_y(radians_from_degrees(30)),
		                    1e-7);

		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("rotate_z", 30).to<Transform>(),
		                    Transform::rotate_z(radians_from_degrees(30)),
		                    1e-7);

		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("rotate", Variant::map("angle", 30, "axis", Variant::vector(1, 2, 3)))
		                            .to<Transform>(),
		                    Transform::rotate(radians_from_degrees(30), {1, 2, 3}),
		                    1e-7);

		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("rotate", Variant::vector(1, 2, 3, 4)).to<Transform>(),
		                    Transform::rotate({1, 2, 3, 4}),
		                    1e-100);

		EXPECT_THROW(Variant::map("rotate", 3).to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantMapLookAt)
	{
		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::map("look_at",
		                                 Variant::map("translation",
		                                              Variant::vector(1, 2, 3),
		                                              "center",
		                                              Variant::vector(4, 5, 6),
		                                              "up",
		                                              Variant::vector(7, 8, 9)))
		                            .to<Transform>(),
		                    Transform::look_at({1, 2, 3}, {4, 5, 6}, {7, 8, 9}),
		                    1e-6);
	}

	TEST(Transform, VariantUnknownType)
	{
		EXPECT_THROW(Variant::map("unknown_type", 0).to<Transform>(), Variant::Exception);
	}

	TEST(Transform, VariantVector)
	{
		EXPECT_PRED_FORMAT3(transform_near,
		                    Variant::vector(Variant::map("scale", 2),
		                                    Variant::map("translate", Variant::vector(10, 20, 30)))
		                            .to<Transform>(),
		                    Transform::combine(Transform::scale(2), Transform::translate(10, 20, 30)),
		                    1e-100);

		EXPECT_THROW(Variant::vector<std::string>({}).to<Transform>(), Variant::Exception);
		EXPECT_THROW(Variant::vector<std::string>({"identity"}).to<Transform>(), Variant::Exception);
	}

	TEST(Transform, Inverse)
	{
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::identity(), 1e-100);

		EXPECT_PRED_FORMAT2(verify_inverse, Transform::translate(1, 2, 3), 1e-100);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::translate({-1, -2, -3}), 1e-100);

		EXPECT_PRED_FORMAT2(verify_inverse, Transform::scale(10, 20, 30), 1e-100);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::scale(-10, -20, -30), 1e-100);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::scale(5), 1e-100);

		EXPECT_PRED_FORMAT2(verify_inverse, Transform::rotate_x(1), 1e-6);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::rotate_y(1), 1e-6);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::rotate_z(1), 1e-6);
		EXPECT_PRED_FORMAT2(verify_inverse,
		                    Transform::rotate(2, {0.5773502588, 0.5773502588, 0.5773502588}),
		                    1e-6);
		EXPECT_PRED_FORMAT2(verify_inverse, Transform::rotate({0.18257, 0.36515, 0.54772, 0.73030}), 1e-5);

		EXPECT_PRED_FORMAT2(verify_inverse,
		                    Transform::combine(Transform::translate(1, 2, 3),
		                                       Transform::rotate({0.18257, 0.36515, 0.54772, 0.73030})),
		                    1e-5);
	}

	TEST(Transform, TransformPoint)
	{
		Transform t = Transform::combine(Transform::rotate_z(PI), Transform::translate(10, 20, 30));

		Vector3 p = t.transform_point({1, 2, 3});
		EXPECT_NEAR(-11, p.x(), 1e-5);
		EXPECT_NEAR(-22, p.y(), 1e-5);
		EXPECT_NEAR(33, p.z(), 1e-5);

		std::vector<Vector3> ps{{4, 5, 6}, {7, 8, 9}};
		t.transform_points(ps);
		EXPECT_NEAR(-14, ps[0].x(), 1e-5);
		EXPECT_NEAR(-25, ps[0].y(), 1e-5);
		EXPECT_NEAR(36, ps[0].z(), 1e-5);
		EXPECT_NEAR(-17, ps[1].x(), 1e-5);
		EXPECT_NEAR(-28, ps[1].y(), 1e-5);
		EXPECT_NEAR(39, ps[1].z(), 1e-5);
	}

	TEST(Transform, TransformDirection)
	{
		Transform t = Transform::combine(Transform::rotate_z(PI), Transform::translate(10, 20, 30));

		Vector3 p = t.transform_direction({1, 2, 3});
		EXPECT_NEAR(-1, p.x(), 1e-6);
		EXPECT_NEAR(-2, p.y(), 1e-6);
		EXPECT_NEAR(3, p.z(), 1e-6);
	}

	TEST(Transform, TransformNormal)
	{
		Transform t = Transform::combine(Transform::rotate_z(PI), Transform::translate(10, 20, 30));

		Vector3 n = t.transform_normal({1, 2, 3});
		EXPECT_NEAR(-0.267261241, n.x(), 1e-7);
		EXPECT_NEAR(-0.534522483, n.y(), 1e-7);
		EXPECT_NEAR(0.801783726, n.z(), 1e-7);

		std::vector<Vector3> ns{{4, 5, 6}, {7, 8, 9}};
		t.transform_normals(ns);
		EXPECT_NEAR(-0.455842256, ns[0].x(), 1e-7);
		EXPECT_NEAR(-0.569802940, ns[0].y(), 1e-7);
		EXPECT_NEAR(0.683763504, ns[0].z(), 1e-7);
		EXPECT_NEAR(-0.502570629, ns[1].x(), 1e-7);
		EXPECT_NEAR(-0.574366510, ns[1].y(), 1e-7);
		EXPECT_NEAR(0.646162271, ns[1].z(), 1e-7);
	}

	TEST(Transform, TransformAABB)
	{
		Transform t = Transform::combine(Transform::rotate_z(PI), Transform::translate(10, 20, 30));

		AABB aabb = t.transform_aabb({{1, 2, 3}, {4, 5, 6}});
		EXPECT_NEAR(-14, aabb.minimum().x(), 1e-5);
		EXPECT_NEAR(-25, aabb.minimum().y(), 1e-5);
		EXPECT_NEAR(33, aabb.minimum().z(), 1e-5);
		EXPECT_NEAR(-11, aabb.maximum().x(), 1e-5);
		EXPECT_NEAR(-22, aabb.maximum().y(), 1e-5);
		EXPECT_NEAR(36, aabb.maximum().z(), 1e-5);
	}

	TEST(Transform, TransformRay)
	{
		Transform t = Transform::combine(Transform::rotate_z(PI), Transform::translate(10, 20, 30));

		Ray ray = t.transform_ray({{1, 2, 3}, {4, 5, 6}, 7});
		EXPECT_NEAR(-11, ray.origin.x(), 1e-5);
		EXPECT_NEAR(-22, ray.origin.y(), 1e-5);
		EXPECT_NEAR(33, ray.origin.z(), 1e-5);
		EXPECT_NEAR(-4, ray.direction.x(), 1e-5);
		EXPECT_NEAR(-5, ray.direction.y(), 1e-5);
		EXPECT_NEAR(6, ray.direction.z(), 1e-5);
		EXPECT_NEAR(7, ray.time, 1e-100);
	}
}
