// This file is part of Rayni.
//
// Copyright (C) 2013-2020 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_AABB_H
#define RAYNI_LIB_MATH_AABB_H

#include <algorithm>
#include <cassert>
#include <limits>

#include "lib/math/math.h"
#include "lib/math/ray.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	class AABB
	{
	public:
		struct Split;

		AABB() = default;

		AABB(const Vector3 &minimum, const Vector3 &maximum) : minimum_(minimum), maximum_(maximum)
		{
		}

		const Vector3 &minimum() const
		{
			return minimum_;
		}

		const Vector3 &maximum() const
		{
			return maximum_;
		}

		AABB &merge(const AABB &aabb)
		{
			minimum_ = Vector3::min(minimum(), aabb.minimum());
			maximum_ = Vector3::max(maximum(), aabb.maximum());
			return *this;
		}

		AABB &merge(const Vector3 &point)
		{
			return merge(AABB(point, point));
		}

		bool intersects(const Ray &ray, real_t &t_min_out, real_t &t_max_out) const
		{
			real_t t_min = 0;
			real_t t_max = std::numeric_limits<real_t>::infinity();

			for (unsigned int i = 0; i < 3; i++)
			{
				real_t inv_ray_dir = 1 / ray.direction[i];
				real_t t_near = (minimum()[i] - ray.origin[i]) * inv_ray_dir;
				real_t t_far = (maximum()[i] - ray.origin[i]) * inv_ray_dir;

				if (t_far < t_near)
					std::swap(t_near, t_far);

				t_far *= 1 + 2 * error_bound_gamma(3);

				t_min = std::max(t_min, t_near);
				t_max = std::min(t_max, t_far);

				if (t_min > t_max)
					return false;
			}

			t_min_out = t_min;
			t_max_out = t_max;

			return true;
		}

		AABB intersection(const AABB &aabb) const
		{
			return {Vector3::max(minimum(), aabb.minimum()), Vector3::min(maximum(), aabb.maximum())};
		}

		real_t surface_area() const
		{
			Vector3 d = maximum() - minimum();
			return 2 * (d.x() * d.y() + d.x() * d.z() + d.y() * d.z());
		}

		Split split(unsigned int axis, real_t pos) const;

		bool is_planar(unsigned int axis) const
		{
			return minimum()[axis] == maximum()[axis];
		}

	private:
		Vector3 minimum_ = Vector3::infinity();
		Vector3 maximum_ = -Vector3::infinity();
	};

	struct AABB::Split
	{
		AABB left, right;
	};

	inline AABB::Split AABB::split(unsigned int axis, real_t pos) const
	{
		assert(pos >= minimum()[axis] && pos <= maximum()[axis]);

		AABB left = *this;
		AABB right = *this;
		left.maximum_[axis] = pos;
		right.minimum_[axis] = pos;

		return {left, right};
	}
}

#endif // RAYNI_LIB_MATH_AABB_H
