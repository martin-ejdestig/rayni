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

#ifndef RAYNI_LIB_MATH_RAY_H
#define RAYNI_LIB_MATH_RAY_H

#include "lib/math/math.h"
#include "lib/math/vector3.h"

namespace Rayni
{
	struct Ray
	{
		Ray(const Vector3 &origin, const Vector3 &direction, real_t time)
		        : origin(origin), direction(direction), time(time)
		{
		}

		Vector3 origin;
		Vector3 direction;
		real_t time;
	};
}

#endif // RAYNI_LIB_MATH_RAY_H
