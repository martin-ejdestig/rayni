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

#ifndef _RAYNI_LIB_COLOR_H_
#define _RAYNI_LIB_COLOR_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>

#include "lib/containers/variant.h"
#include "lib/math/math.h"

namespace Rayni
{
	class Color
	{
	public:
		Color()
		{
		}

		constexpr Color(real_t r, real_t g, real_t b) : r_(r), g_(g), b_(b)
		{
		}

		static constexpr Color black()
		{
			return Color(0, 0, 0);
		}

		static constexpr Color white()
		{
			return Color(1, 1, 1);
		}

		static constexpr Color red()
		{
			return Color(1, 0, 0);
		}

		static constexpr Color yellow()
		{
			return Color(1, 1, 0);
		}

		static constexpr Color green()
		{
			return Color(0, 1, 0);
		}

		static constexpr Color blue()
		{
			return Color(0, 0, 1);
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
			return Color(r() + c.r(), g() + c.g(), b() + c.b());
		}

		Color operator-(const Color &v) const
		{
			return Color(r() - v.r(), g() - v.g(), b() - v.b());
		}

		Color &operator+=(const Color &c)
		{
			r() += c.r();
			g() += c.g();
			b() += c.b();

			return *this;
		}

		Color operator*(const Color &c) const
		{
			return Color(r() * c.r(), g() * c.g(), b() * c.b());
		}

		Color &operator*=(const Color &c)
		{
			r() *= c.r();
			g() *= c.g();
			b() *= c.b();

			return *this;
		}

		Color operator*(real_t s) const
		{
			return Color(r() * s, g() * s, b() * s);
		}

		friend Color operator*(real_t s, const Color &c)
		{
			return Color(s * c.r(), s * c.g(), s * c.b());
		}

		Color &operator*=(real_t s)
		{
			r() *= s;
			g() *= s;
			b() *= s;

			return *this;
		}

		bool operator==(const Color &c) const
		{
			constexpr real_t COMPONENT_MAX_DIFF = real_t(0.001);
			return std::abs(r() - c.r()) <= COMPONENT_MAX_DIFF &&
			       std::abs(g() - c.g()) <= COMPONENT_MAX_DIFF &&
			       std::abs(b() - c.b()) <= COMPONENT_MAX_DIFF;
		}

		Color clamp() const
		{
			return Color(std::min(std::max(r(), real_t(0)), real_t(1)),
			             std::min(std::max(g(), real_t(0)), real_t(1)),
			             std::min(std::max(b(), real_t(0)), real_t(1)));
		}

	private:
		real_t r_ = 0, g_ = 0, b_ = 0;
	};

	static inline std::ostream &operator<<(std::ostream &ostream, const Color &color)
	{
		ostream << "(" << color.r() << "," << color.g() << "," << color.b() << ")";
		return ostream;
	}
}

#endif // _RAYNI_LIB_COLOR_H_
