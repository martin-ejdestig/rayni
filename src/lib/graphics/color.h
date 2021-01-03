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

#ifndef RAYNI_LIB_GRAPHICS_COLOR_H
#define RAYNI_LIB_GRAPHICS_COLOR_H

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "lib/containers/variant.h"
#include "lib/math/math.h"

namespace Rayni
{
	class Color
	{
	public:
		Color() = default;

		constexpr Color(real_t r, real_t g, real_t b) : r_(r), g_(g), b_(b)
		{
		}

		static constexpr Color black()
		{
			return {0, 0, 0};
		}

		static constexpr Color white()
		{
			return {1, 1, 1};
		}

		static constexpr Color red()
		{
			return {1, 0, 0};
		}

		static constexpr Color yellow()
		{
			return {1, 1, 0};
		}

		static constexpr Color green()
		{
			return {0, 1, 0};
		}

		static constexpr Color blue()
		{
			return {0, 0, 1};
		}

		static Color from_variant(const Variant &v);

		real_t r() const
		{
			return r_;
		}

		real_t &r()
		{
			return r_;
		}

		real_t g() const
		{
			return g_;
		}

		real_t &g()
		{
			return g_;
		}

		real_t b() const
		{
			return b_;
		}

		real_t &b()
		{
			return b_;
		}

		Color operator+(const Color &c) const
		{
			return {r_ + c.r_, g_ + c.g_, b_ + c.b_};
		}

		Color operator-(const Color &c) const
		{
			return {r_ - c.r_, g_ - c.g_, b_ - c.b_};
		}

		Color &operator+=(const Color &c)
		{
			r_ += c.r_;
			g_ += c.g_;
			b_ += c.b_;

			return *this;
		}

		Color operator*(const Color &c) const
		{
			return {r_ * c.r_, g_ * c.g_, b_ * c.b_};
		}

		Color &operator*=(const Color &c)
		{
			r_ *= c.r_;
			g_ *= c.g_;
			b_ *= c.b_;

			return *this;
		}

		Color operator*(real_t s) const
		{
			return {r_ * s, g_ * s, b_ * s};
		}

		friend Color operator*(real_t s, const Color &c)
		{
			return {s * c.r_, s * c.g_, s * c.b_};
		}

		Color &operator*=(real_t s)
		{
			r_ *= s;
			g_ *= s;
			b_ *= s;

			return *this;
		}

		Color clamp() const
		{
			return {std::clamp(r_, real_t(0), real_t(1)),
			        std::clamp(g_, real_t(0), real_t(1)),
			        std::clamp(b_, real_t(0), real_t(1))};
		}

	private:
		real_t r_ = 0, g_ = 0, b_ = 0;
	};
}

#endif // RAYNI_LIB_GRAPHICS_COLOR_H
