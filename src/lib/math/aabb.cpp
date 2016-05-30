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

#include "lib/math/aabb.h"

#include <algorithm>
#include <limits>

#include "lib/math/ray.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	bool AABB::intersects(const Ray &ray, real_t &t_min_out, real_t &t_max_out) const
	{
		real_t t_min = 0;
		real_t t_max = std::numeric_limits<real_t>::infinity();

		for (unsigned int i = 0; i < 3; i++)
		{
			real_t inv_ray_dir = 1 / ray.direction[i];
			real_t t_near = (minimum[i] - ray.origin[i]) * inv_ray_dir;
			real_t t_far = (maximum[i] - ray.origin[i]) * inv_ray_dir;

			if (t_far < t_near)
				std::swap(t_near, t_far);

			t_min = std::max(t_min, t_near);
			t_max = std::min(t_max, t_far);

			if (t_min > t_max)
				return false;
		}

		t_min_out = t_min;
		t_max_out = t_max;

		return true;
	}
}
