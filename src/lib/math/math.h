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

#ifndef RAYNI_LIB_MATH_MATH_H
#define RAYNI_LIB_MATH_MATH_H

#include <cmath>
#include <cstdint>
#include <limits>

#include "config.h"

namespace Rayni
{
#if RAYNI_DOUBLE_PRECISION
	using real_t = double;
	using real_uint_t = std::uint64_t;
#else
	using real_t = float;
	using real_uint_t = std::uint32_t;
#endif
	static_assert(sizeof(real_t) == sizeof(real_uint_t));
	static_assert(std::numeric_limits<real_t>::is_iec559, "real_t is not a IEEE 754 float");

	static constexpr real_t REAL_MAX = std::numeric_limits<real_t>::max();
	static constexpr real_t REAL_LOWEST = std::numeric_limits<real_t>::lowest();
	static constexpr real_t REAL_EPSILON = std::numeric_limits<real_t>::epsilon();
	static constexpr real_t REAL_INFINITY = std::numeric_limits<real_t>::infinity();

	static constexpr real_t PI = real_t(3.14159265358979323846);

	static constexpr inline real_t frac(real_t x)
	{
		return x - std::floor(x);
	}

	static constexpr inline int ifloor(real_t x)
	{
		return static_cast<int>(std::floor(x));
	}

	static constexpr inline real_t radians_from_degrees(real_t degrees)
	{
		return degrees * PI / 180;
	}

	static constexpr inline real_uint_t real_to_uint(real_t r)
	{
		union
		{
			real_t r;
			real_uint_t i;
		} u;
		u.r = r;
		return u.i;
	}

	static constexpr inline real_t real_from_uint(real_uint_t i)
	{
		union
		{
			real_t r;
			real_uint_t i;
		} u;
		u.i = i;
		return u.r;
	}

	// Next number > r.
	//
	// Same as std::nextafter(r, infinity) but simpler and inlined (std::nextafter is not
	// inlined with GCC/libstdc++/glibc at least).
	//
	// If r is positive or 0 increase mantissa else decrease it. Since IEEE 754 is assumed,
	// exponent will be increased/decreased accordingly on mantissa over/underflow. If r is
	// maximum value it will also result in infinity (e.g. float max: 0x7f7fffff,
	// infinity: 0x7f800000, that is all bits in mantissa cleared and all bits in exponent set).
	static constexpr inline real_t real_next_up(real_t r)
	{
		if (r > REAL_MAX)
			return r; // +infinity. Detect with > max since std::isinf() is a NOP with -ffast-math.

		if (r == -real_t(0))
			r = 0; // +0 and -0 bit patterns are not adjacent, -0 => +0

		real_uint_t i = real_to_uint(r);

		if (r >= 0)
			++i;
		else
			--i;

		return real_from_uint(i);
	}

	// Next number < r.
	//
	// Same as std::nextafter(r, -infinity) but simpler and inlined (std::nextafter is not
	// inlined with GCC/libstdc++/glibc at least).
	//
	// If r is positive decrease mantissa else increase it. Since IEEE 754 is assumed,
	// exponent will be increased/decreased accordingly on mantissa over/underflow. If r is
	// lowest value it will also result in -infinity (e.g. float min: 0xff7fffff,
	// -infinity: 0xff800000, that is all bits in mantissa cleared, all bits in exponent set
	// and sign bit set).
	static constexpr inline real_t real_next_down(real_t r)
	{
		if (r < REAL_LOWEST)
			return r; // -infinity. Detect with < lowest since std::isinf() is a NOP with -ffast-math.

		if (r == 0)
			r = -real_t(0); // -0 and +0 bit patterns are not adjacent, +0 => -0

		real_uint_t i = real_to_uint(r);

		if (r > 0)
			--i;
		else
			++i;

		return real_from_uint(i);
	}

	// See Pharr, M, Jakob, W, and Humphreys, G 2016. Physically Based Rendering. 3rd ed.
	// Chapter 3.9, Managing Rounding Error, p. 206-236.
	static constexpr inline real_t error_bound_gamma(unsigned int n)
	{
		constexpr real_t MACHINE_EPSILON = REAL_EPSILON / 2;

		return (real_t(n) * MACHINE_EPSILON) / (1 - real_t(n) * MACHINE_EPSILON);
	}
}

#endif // RAYNI_LIB_MATH_MATH_H
