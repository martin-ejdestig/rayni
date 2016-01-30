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

#include <utility>

#include "lib/containers/variant.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	TEST(Vector3Test, Variant)
	{
		auto vector_variant = [](unsigned int size)
		{
			Variant::Vector v;
			for (unsigned int i = 0; i < size; i++)
				v.emplace_back(i + 1);
			return Variant(std::move(v));
		};

		Vector3 v3 = vector_variant(3).to<Vector3>();
		EXPECT_NEAR(1, v3.x(), 1e-100);
		EXPECT_NEAR(2, v3.y(), 1e-100);
		EXPECT_NEAR(3, v3.z(), 1e-100);

		EXPECT_THROW(vector_variant(0).to<Vector3>(), Variant::Exception);
		EXPECT_THROW(vector_variant(1).to<Vector3>(), Variant::Exception);
		EXPECT_THROW(vector_variant(2).to<Vector3>(), Variant::Exception);
		EXPECT_NO_THROW(vector_variant(4).to<Vector3>()); // TODO: Reconsider?

		EXPECT_THROW(Variant(0).to<Vector3>(), Variant::Exception);
	}

	TEST(Vector3Test, ComponentMinMax)
	{
		EXPECT_NEAR(1, Vector3::min({1, 2, 2}, {2, 2, 2}).x(), 1e-100);
		EXPECT_NEAR(1, Vector3::min({2, 1, 2}, {2, 2, 2}).y(), 1e-100);
		EXPECT_NEAR(1, Vector3::min({2, 2, 1}, {2, 2, 2}).z(), 1e-100);
		EXPECT_NEAR(1, Vector3::min({2, 2, 2}, {1, 2, 2}).x(), 1e-100);
		EXPECT_NEAR(1, Vector3::min({2, 2, 2}, {2, 1, 2}).y(), 1e-100);
		EXPECT_NEAR(1, Vector3::min({2, 2, 2}, {2, 2, 1}).z(), 1e-100);

		EXPECT_NEAR(2, Vector3::max({2, 1, 1}, {1, 1, 1}).x(), 1e-100);
		EXPECT_NEAR(2, Vector3::max({1, 2, 1}, {1, 1, 1}).y(), 1e-100);
		EXPECT_NEAR(2, Vector3::max({1, 1, 2}, {1, 1, 1}).z(), 1e-100);
		EXPECT_NEAR(2, Vector3::max({1, 1, 1}, {2, 1, 1}).x(), 1e-100);
		EXPECT_NEAR(2, Vector3::max({1, 1, 1}, {1, 2, 1}).y(), 1e-100);
		EXPECT_NEAR(2, Vector3::max({1, 1, 1}, {1, 1, 2}).z(), 1e-100);
	}

	TEST(Vector3Test, Operators)
	{
		Vector3 v;

		v = Vector3(1, 2, 3);
		EXPECT_EQ(v.x(), v[0]);
		EXPECT_EQ(v.y(), v[1]);
		EXPECT_EQ(v.z(), v[2]);

		const Vector3 vc(1, 2, 3);
		EXPECT_EQ(vc.x(), v[0]);
		EXPECT_EQ(vc.y(), v[1]);
		EXPECT_EQ(vc.z(), v[2]);

		v = Vector3(1, 2, 3) + Vector3(4, 5, 6);
		EXPECT_NEAR(5, v.x(), 1e-100);
		EXPECT_NEAR(7, v.y(), 1e-100);
		EXPECT_NEAR(9, v.z(), 1e-100);

		v = Vector3(6, 5, 4) - Vector3(1, 2, 3);
		EXPECT_NEAR(5, v.x(), 1e-100);
		EXPECT_NEAR(3, v.y(), 1e-100);
		EXPECT_NEAR(1, v.z(), 1e-100);

		v = -Vector3(1, 2, 3);
		EXPECT_EQ(-1, v.x());
		EXPECT_EQ(-2, v.y());
		EXPECT_EQ(-3, v.z());

		v = Vector3(1, 2, 3) * real_t(2);
		EXPECT_NEAR(2, v.x(), 1e-100);
		EXPECT_NEAR(4, v.y(), 1e-100);
		EXPECT_NEAR(6, v.z(), 1e-100);

		v = real_t(2) * Vector3(3, 2, 1);
		EXPECT_NEAR(6, v.x(), 1e-100);
		EXPECT_NEAR(4, v.y(), 1e-100);
		EXPECT_NEAR(2, v.z(), 1e-100);

		v = Vector3(1, 2, 3);
		v += Vector3(4, 5, 6);
		EXPECT_NEAR(5, v.x(), 1e-100);
		EXPECT_NEAR(7, v.y(), 1e-100);
		EXPECT_NEAR(9, v.z(), 1e-100);

		v = Vector3(6, 5, 4);
		v -= Vector3(1, 2, 3);
		EXPECT_NEAR(5, v.x(), 1e-100);
		EXPECT_NEAR(3, v.y(), 1e-100);
		EXPECT_NEAR(1, v.z(), 1e-100);
	}

	TEST(Vector3Test, Normalize)
	{
		EXPECT_NEAR(1, Vector3(2, 0, 0).normalize().x(), 1e-100);
		EXPECT_NEAR(1, Vector3(0, 2, 0).normalize().y(), 1e-100);
		EXPECT_NEAR(1, Vector3(0, 0, 2).normalize().z(), 1e-100);
		EXPECT_NEAR(-1, Vector3(-2, 0, 0).normalize().x(), 1e-100);
		EXPECT_NEAR(-1, Vector3(0, -2, 0).normalize().y(), 1e-100);
		EXPECT_NEAR(-1, Vector3(0, 0, -2).normalize().z(), 1e-100);

		Vector3 v = Vector3(2, -2, 2).normalize();
		EXPECT_NEAR(0.5773502588, v.x(), 1e-7);
		EXPECT_NEAR(-0.5773502588, v.y(), 1e-7);
		EXPECT_NEAR(0.5773502588, v.z(), 1e-7);
	}

	TEST(Vector3Test, Dot)
	{
		EXPECT_NEAR(3200, Vector3(10, 20, 30).dot(Vector3(40, 50, 60)), 1e-100);
		EXPECT_NEAR(0, Vector3(0, 0, 0).dot(Vector3(40, 50, 60)), 1e-100);
		EXPECT_NEAR(0, Vector3(10, 20, 30).dot(Vector3(0, 0, 0)), 1e-100);
	}

	TEST(Vector3Test, Cross)
	{
		Vector3 v;

		v = Vector3(1, 2, 0).cross(Vector3(3, 4, 0));
		EXPECT_NEAR(0, v.x(), 1e-100);
		EXPECT_NEAR(0, v.y(), 1e-100);
		EXPECT_NEAR(-2, v.z(), 1e-100);

		v = Vector3(3, 4, 0).cross(Vector3(1, 2, 0));
		EXPECT_NEAR(0, v.x(), 1e-100);
		EXPECT_NEAR(0, v.y(), 1e-100);
		EXPECT_NEAR(2, v.z(), 1e-100);

		v = Vector3(0, 1, 2).cross(Vector3(0, 3, 4));
		EXPECT_NEAR(-2, v.x(), 1e-100);
		EXPECT_NEAR(0, v.y(), 1e-100);
		EXPECT_NEAR(0, v.z(), 1e-100);

		v = Vector3(0, 3, 4).cross(Vector3(0, 1, 2));
		EXPECT_NEAR(2, v.x(), 1e-100);
		EXPECT_NEAR(0, v.y(), 1e-100);
		EXPECT_NEAR(0, v.z(), 1e-100);

		v = Vector3(1, 0, 2).cross(Vector3(3, 0, 4));
		EXPECT_NEAR(0, v.x(), 1e-100);
		EXPECT_NEAR(2, v.y(), 1e-100);
		EXPECT_NEAR(0, v.z(), 1e-100);

		v = Vector3(3, 0, 4).cross(Vector3(1, 0, 2));
		EXPECT_NEAR(0, v.x(), 1e-100);
		EXPECT_NEAR(-2, v.y(), 1e-100);
		EXPECT_NEAR(0, v.z(), 1e-100);
	}

	TEST(Vector3Test, Reflect)
	{
		Vector3 v;

		v = Vector3(0.7071099877, 0.7071099877, 0).reflect({0, 1, 0});
		EXPECT_NEAR(0.7071099877, v.x(), 1e-10);
		EXPECT_NEAR(-0.7071099877, v.y(), 1e-10);
		EXPECT_NEAR(0, v.z(), 1e-100);

		v = Vector3(0.7071099877, -0.7071099877, 0).reflect({0, 1, 0});
		EXPECT_NEAR(0.7071099877, v.x(), 1e-10);
		EXPECT_NEAR(0.7071099877, v.y(), 1e-10);
		EXPECT_NEAR(0, v.z(), 1e-100);
	}

	TEST(Vector3Test, Hash)
	{
		EXPECT_TRUE(Vector3(1, 0, 0).hash() != Vector3(0, 1, 0).hash());
		EXPECT_TRUE(Vector3(0, 1, 0).hash() != Vector3(0, 0, 1).hash());
		EXPECT_TRUE(Vector3(0, 0, 1).hash() != Vector3(1, 0, 0).hash());
	}

	TEST(Vector3Test, Compare)
	{
		EXPECT_EQ(0, Vector3::compare({1, 2, 3}, {1, 2, 3}));

		EXPECT_EQ(1, Vector3::compare({1, 2, 3}, {0, 2, 3}));
		EXPECT_EQ(1, Vector3::compare({1, 2, 3}, {1, 0, 3}));
		EXPECT_EQ(1, Vector3::compare({1, 2, 3}, {1, 2, 0}));

		EXPECT_EQ(-1, Vector3::compare({0, 2, 3}, {1, 2, 3}));
		EXPECT_EQ(-1, Vector3::compare({1, 0, 3}, {1, 2, 3}));
		EXPECT_EQ(-1, Vector3::compare({1, 2, 0}, {1, 2, 3}));
	}
}
