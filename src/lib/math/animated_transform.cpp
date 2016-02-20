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

#include <algorithm>

#include "lib/math/aabb.h"
#include "lib/math/animated_transform.h"
#include "lib/math/math.h"

namespace Rayni
{
	AnimatedTransform::AnimatedTransform(real_t start_time,
	                                     const Transform &start_transform,
	                                     real_t end_time,
	                                     const Transform &end_transform)
	        : start_time(start_time),
	          start(start_transform.get_matrix()),
	          end_time(end_time),
	          end(end_transform.get_matrix())
	{
	}

	AnimatedTransform::AnimatedTransform(const Variant &v)
	        : AnimatedTransform(v.get<real_t>("start_time"),
	                            v.get<Transform>("start_transform"),
	                            v.get<real_t>("end_time"),
	                            v.get<Transform>("end_transform"))
	{
		if (start_time >= end_time)
			throw Variant::Exception(v, "start_time >= end_time");
	}

	Transform AnimatedTransform::interpolate(real_t time) const
	{
		real_t ct = std::min(std::max(time, start_time), end_time);
		real_t dt = (ct - start_time) / (end_time - start_time);
		Matrix4x4 m = start.interpolate(dt, end).compose();

		return Transform(m, m.inverse());
	}

	AABB AnimatedTransform::motion_bounds(const AABB &aabb) const
	{
		// TODO: Find maxima and minima instead of stepping
		const unsigned int STEPS = 256;
		AABB bounds;

		for (unsigned int i = 0; i < STEPS; i++)
		{
			real_t time = lerp(real_t(i) / (STEPS - 1), start_time, end_time);
			bounds.merge(interpolate(time).transform_aabb(aabb));
		}

		return bounds;
	}
}
