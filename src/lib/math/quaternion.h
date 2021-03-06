// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_QUATERNION_H
#define RAYNI_LIB_MATH_QUATERNION_H

#include <array>
#include <cmath>

#include "lib/containers/variant.h"
#include "lib/function/result.h"
#include "lib/math/math.h"

namespace Rayni
{
	class Quaternion
	{
	public:
		Quaternion() = default;

		constexpr Quaternion(real_t x, real_t y, real_t z, real_t w) : x_(x), y_(y), z_(z), w_(w)
		{
		}

		constexpr explicit Quaternion(const std::array<real_t, 4> &xyzw) :
		        Quaternion(xyzw[0], xyzw[1], xyzw[2], xyzw[3])
		{
		}

		static Result<Quaternion> from_variant(const Variant &v);

		real_t x() const
		{
			return x_;
		}

		real_t &x()
		{
			return x_;
		}

		real_t y() const
		{
			return y_;
		}

		real_t &y()
		{
			return y_;
		}

		real_t z() const
		{
			return z_;
		}

		real_t &z()
		{
			return z_;
		}

		real_t w() const
		{
			return w_;
		}

		real_t &w()
		{
			return w_;
		}

		Quaternion operator+(const Quaternion &q) const
		{
			return {x() + q.x(), y() + q.y(), z() + q.z(), w() + q.w()};
		}

		Quaternion operator-(const Quaternion &q) const
		{
			return {x() - q.x(), y() - q.y(), z() - q.z(), w() - q.w()};
		}

		Quaternion operator*(real_t s) const
		{
			return {x() * s, y() * s, z() * s, w() * s};
		}

		friend Quaternion operator*(real_t s, const Quaternion &q)
		{
			return {s * q.x(), s * q.y(), s * q.z(), s * q.w()};
		}

		Quaternion normalize() const
		{
			real_t len_inv = 1 / std::sqrt(dot(*this));
			return *this * len_inv;
		}

		real_t dot(const Quaternion &q) const
		{
			return x() * q.x() + y() * q.y() + z() * q.z() + w() * q.w();
		}

	private:
		real_t x_ = 0;
		real_t y_ = 0;
		real_t z_ = 0;
		real_t w_ = 0;
	};
}

#endif // RAYNI_LIB_MATH_QUATERNION_H
