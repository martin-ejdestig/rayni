/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_VECTOR4_H
#define RAYNI_LIB_MATH_VECTOR4_H

#include <cassert>

#include "lib/math/math.h"

namespace Rayni
{
	class Vector4
	{
	public:
		static constexpr unsigned int SIZE = 4;

		Vector4()
		{
		}

		constexpr Vector4(real_t x, real_t y, real_t z, real_t w) : xyzw_{x, y, z, w}
		{
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

		real_t w() const
		{
			return (*this)[3];
		}

		real_t &w()
		{
			return (*this)[3];
		}

		real_t operator[](unsigned int i) const
		{
			assert(i < SIZE);
			return xyzw_[i];
		}

		real_t &operator[](unsigned int i)
		{
			assert(i < SIZE);
			return xyzw_[i];
		}

		Vector4 operator*(real_t s) const
		{
			return {x() * s, y() * s, z() * s, w() * s};
		}

		Vector4 &operator+=(const Vector4 &v)
		{
			x() += v.x();
			y() += v.y();
			z() += v.z();
			w() += v.w();

			return *this;
		}

		Vector4 &operator*=(real_t s)
		{
			x() *= s;
			y() *= s;
			z() *= s;
			w() *= s;

			return *this;
		}

		real_t dot(const Vector4 &v) const
		{
			return x() * v.x() + y() * v.y() + z() * v.z() + w() * v.w();
		}

	private:
		real_t xyzw_[SIZE] = {0, 0, 0, 0};
	};
}

#endif // RAYNI_LIB_MATH_VECTOR4_H
