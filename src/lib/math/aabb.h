/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_MATH_AABB_H_
#define _RAYNI_LIB_MATH_AABB_H_

#include <cassert>
#include <iostream>

#include "lib/math/math.h"
#include "lib/math/ray.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class AABB
	{
	public:
		struct Split;

		AABB()
		{
		}

		AABB(const Vector3 &minimum, const Vector3 &maximum) : minimum(minimum), maximum(maximum)
		{
		}

		const Vector3 &get_minimum() const
		{
			return minimum;
		}

		const Vector3 &get_maximum() const
		{
			return maximum;
		}

		AABB &merge(const AABB &aabb)
		{
			minimum = Vector3::min(minimum, aabb.minimum);
			maximum = Vector3::max(maximum, aabb.maximum);
			return *this;
		}

		AABB &merge(const Vector3 &point)
		{
			return merge(AABB(point, point));
		}

		// TODO: Want to return an optional struct with t_min and t_max as members. But
		//       both GCC (5.3.0 with libstdc++) and Clang (3.7.1 with libc++) generate
		//       worse code in KdTree and BVH intersection methods if this is done, even
		//       though there really is no reason for it. Continue using out arguments
		//       for now since the intersection methods are so performance critical,
		bool intersects(const Ray &ray, real_t &t_min, real_t &t_max) const;

		AABB intersection(const AABB &aabb) const
		{
			return AABB(Vector3::max(minimum, aabb.minimum), Vector3::min(maximum, aabb.maximum));
		}

		real_t surface_area() const
		{
			Vector3 d = maximum - minimum;
			return 2 * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
		}

		Split split(unsigned int axis, real_t pos) const;

		bool is_planar(unsigned int axis) const
		{
			return minimum[axis] == maximum[axis];
		}

	private:
		Vector3 minimum = Vector3::infinity();
		Vector3 maximum = -Vector3::infinity();
	};

	struct AABB::Split
	{
		AABB left, right;
	};

	inline AABB::Split AABB::split(unsigned int axis, real_t pos) const
	{
		assert(pos >= minimum[axis] && pos <= maximum[axis]);

		AABB left = *this, right = *this;
		left.maximum[axis] = pos;
		right.minimum[axis] = pos;

		return {left, right};
	}
}

#endif // _RAYNI_LIB_MATH_AABB_H_
