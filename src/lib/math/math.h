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

#ifndef _RAYNI_LIB_MATH_MATH_H_
#define _RAYNI_LIB_MATH_MATH_H_

#include <cmath>

#include "config.h"

namespace Rayni
{
#ifdef RAYNI_DOUBLE_PRECISION
	typedef double real_t;
#else
	typedef float real_t;
#endif

	static constexpr real_t PI = real_t(3.14159265358979323846);

	template <typename T>
	static inline T lerp(real_t t, const T &x0, const T &x1)
	{
		return x0 + t * (x1 - x0);
	}

	template <typename T>
	static inline T blerp(real_t tx, real_t ty, const T &x00, const T &x10, const T &x01, const T &x11)
	{
		return lerp(ty, lerp(tx, x00, x10), lerp(tx, x01, x11));
	}

	static inline real_t frac(real_t x)
	{
		return x - std::floor(x);
	}

	static inline int ifloor(real_t x)
	{
		return static_cast<int>(std::floor(x));
	}

	static inline real_t radians_from_degrees(real_t degrees)
	{
		return degrees * PI / 180;
	}
}

#endif // _RAYNI_LIB_MATH_MATH_H_
