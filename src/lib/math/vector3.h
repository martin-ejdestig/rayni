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

#ifndef RAYNI_LIB_MATH_VECTOR3_H
#define RAYNI_LIB_MATH_VECTOR3_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>

#include "lib/containers/variant.h"
#include "lib/math/hash.h"
#include "lib/math/math.h"

namespace Rayni
{
	class Vector3 // NOLINT Remove when https://llvm.org/bugs/show_bug.cgi?id=30965 is fixed.
	{
	public:
		static constexpr unsigned int SIZE = 3;

		Vector3()
		{
		}

		constexpr Vector3(real_t x, real_t y, real_t z) : xyz{x, y, z}
		{
		}

		constexpr explicit Vector3(const std::array<real_t, 3> &xyz) : Vector3(xyz[0], xyz[1], xyz[2])
		{
		}

		explicit Vector3(const Variant &v) : Vector3(v.get<real_t>(0), v.get<real_t>(1), v.get<real_t>(2))
		{
		}

		static constexpr Vector3 infinity()
		{
			return Vector3(std::numeric_limits<real_t>::infinity(),
			               std::numeric_limits<real_t>::infinity(),
			               std::numeric_limits<real_t>::infinity());
		}

		static Vector3 min(const Vector3 &a, const Vector3 &b)
		{
			return Vector3(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()));
		}

		static Vector3 max(const Vector3 &a, const Vector3 &b)
		{
			return Vector3(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
		}

		real_t x() const
		{
			return (*this)[0];
		}

		real_t &x()
		{
			return (*this)[0];
		}

		real_t y() const
		{
			return (*this)[1];
		}

		real_t &y()
		{
			return (*this)[1];
		}

		real_t z() const
		{
			return (*this)[2];
		}

		real_t &z()
		{
			return (*this)[2];
		}

		real_t operator[](unsigned int i) const
		{
			assert(i < SIZE);
			return xyz[i];
		}

		real_t &operator[](unsigned int i)
		{
			assert(i < SIZE);
			return xyz[i];
		}

		Vector3 operator+(const Vector3 &v) const
		{
			return Vector3(x() + v.x(), y() + v.y(), z() + v.z());
		}

		Vector3 operator-(const Vector3 &v) const
		{
			return Vector3(x() - v.x(), y() - v.y(), z() - v.z());
		}

		Vector3 operator-() const
		{
			return Vector3(-x(), -y(), -z());
		}

		Vector3 operator*(real_t s) const
		{
			return Vector3(x() * s, y() * s, z() * s);
		}

		friend Vector3 operator*(real_t s, const Vector3 &v)
		{
			return Vector3(s * v.x(), s * v.y(), s * v.z());
		}

		Vector3 &operator+=(const Vector3 &v)
		{
			x() += v.x();
			y() += v.y();
			z() += v.z();

			return *this;
		}

		Vector3 &operator-=(const Vector3 &v)
		{
			x() -= v.x();
			y() -= v.y();
			z() -= v.z();

			return *this;
		}

		Vector3 &operator*=(real_t s)
		{
			x() *= s;
			y() *= s;
			z() *= s;

			return *this;
		}

		Vector3 normalize() const
		{
			real_t len_inv = 1 / std::sqrt(dot(*this));
			return *this * len_inv;
		}

		real_t dot(const Vector3 &v) const
		{
			return x() * v.x() + y() * v.y() + z() * v.z();
		}

		Vector3 cross(const Vector3 &v) const
		{
			return Vector3(y() * v.z() - z() * v.y(), z() * v.x() - x() * v.z(), x() * v.y() - y() * v.x());
		}

		Vector3 reflect(const Vector3 &normal) const
		{
			return *this - normal * dot(normal) * 2.0;
		}

		std::size_t hash() const
		{
			return hash_combine_for(x(), y(), z());
		}

		static inline int compare(const Vector3 &v1, const Vector3 &v2)
		{
			if (v1.x() < v2.x())
				return -1;
			if (v1.x() > v2.x())
				return 1;
			if (v1.y() < v2.y())
				return -1;
			if (v1.y() > v2.y())
				return 1;
			if (v1.z() < v2.z())
				return -1;
			if (v1.z() > v2.z())
				return 1;
			return 0;
		}

	private:
		real_t xyz[SIZE] = {0, 0, 0};
	};
}

#endif // RAYNI_LIB_MATH_VECTOR3_H
