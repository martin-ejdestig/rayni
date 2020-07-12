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

#ifndef RAYNI_LIB_MATH_RAY_H
#define RAYNI_LIB_MATH_RAY_H

#include "lib/math/math.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	struct Ray
	{
		Ray(const Vector3 &origin_in, const Vector3 &direction_in, real_t time_in) :
		        origin(origin_in),
		        direction(direction_in),
		        time(time_in)
		{
		}

		Vector3 origin;
		Vector3 direction;
		real_t time;
	};

	// Generate ray from origin taking error used to calculate origin into account to avoid
	// self-intersection.
	//
	// See Pharr, M, Jakob, W, and Humphreys, G 2016. Physically Based Rendering. 3rd ed.
	// Chapter 3.9.5, Robust Spawned Ray Origins, p. 230-233.
	//
	// TODO: d += 0.0001 should not be needed (e.g. PBRT does not add fixed offset). But
	// currently, without this, many rays intersect when spawned from intersection even though
	// they should not. An example of this can be seen in Boing Ball scene at pixel x=463,y=208
	// when rendered in 1280x720. A black dot occurrs at plane that has y=0 in world space.
	// Normal (0, 1, 0) and origin_error is (ex, 0, ey) (see TriangleMesh::Triangle::intersect()).
	// d in this case becomes (0, 1, 0) * (ex, 0, ey) = 0 so origin will not be offset. Rays
	// towards light sources will thus intersect with the plane (point is in shadow). Probably
	// not a problem in PBRT since when checking if ray t > 0, error bounds for calculation of t
	// is taken into account (see 3.9.6 Avoiding Intersections Behind Ray/ Origins). Should
	// probably do the same in Rayni so this hardcoded offset can be removed.
	//
	// TODO: What do other renderers do to handle this? Is there a cheaper way than taking error
	// bounds into account everywhere that still gives good results? Only having a hardcoded
	// offset along normal is what Rayni has done since day one. That proved to be insufficient
	// though. There were a lot of black dots in Stanford Lucy and Sponza Outside scenes before
	// taking origin error bounds into account when offsetting ray.
	static inline Ray ray_with_offset_origin(const Vector3 &origin,
	                                         const Vector3 &origin_error,
	                                         const Vector3 &direction,
	                                         const Vector3 &normal,
	                                         real_t time)
	{
		real_t d = normal.abs().dot(origin_error);
		d += real_t(0.0001); // Remove, see TODO above.

		Vector3 offset = d * normal;

		if (direction.dot(normal) < 0)
			offset = -offset;

		Vector3 offset_origin = origin + offset;

		for (unsigned int i = 0; i < 3; ++i)
		{
			if (offset[i] > 0)
				offset_origin[i] = real_next_up(offset_origin[i]);
			else if (offset[i] < 0)
				offset_origin[i] = real_next_down(offset_origin[i]);
		}

		return {offset_origin, direction, time};
	}
}

#endif // RAYNI_LIB_MATH_RAY_H
