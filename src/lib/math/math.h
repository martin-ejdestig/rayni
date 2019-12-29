// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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
#include <limits>

#include "config.h"

namespace Rayni
{
#ifdef RAYNI_DOUBLE_PRECISION
	using real_t = double;
#else
	using real_t = float;
#endif

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

	// See Pharr, M, Jakob, W, and Humphreys, G 2016. Physically Based Rendering. 3rd ed.
	// Chapter 3.9, Managing Rounding Error, p. 206-236.
	static constexpr inline real_t error_bound_gamma(unsigned int n)
	{
		constexpr real_t MACHINE_EPSILON = std::numeric_limits<real_t>::epsilon() / 2;

		return (real_t(n) * MACHINE_EPSILON) / (1 - real_t(n) * MACHINE_EPSILON);
	}
}

#endif // RAYNI_LIB_MATH_MATH_H
