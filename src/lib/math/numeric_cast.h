// This file is part of Rayni.
//
// Copyright (C) 2018-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_MATH_NUMERIC_CAST_H
#define RAYNI_LIB_MATH_NUMERIC_CAST_H

#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>

namespace Rayni
{
	// Cast value of type V to value of type T if value is withing range of T.
	//
	// Floating point values are rounded to the nearest integer, rounding halfway cases away
	// from zero.
	//
	// NOTE: C++'s math error handling can not be used since Rayni is compiled with -ffast-math
	//       in release mode. When changing this function, make sure all numeric_cast() tests
	//       pass in both release (-ffast-math) and debug mode (no -ffast-math).
	template <typename T, typename V>
	constexpr std::optional<T> numeric_cast(V v)
	{
		static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
		static_assert(std::is_integral_v<V> || std::is_floating_point_v<V>);

		[[maybe_unused]] auto float_to_integer = [](auto f) { return std::llround(f); };

		auto within_range = [&] {
			if constexpr (std::is_same_v<T, V>)
				return true;

			using C = std::common_type_t<T, V>;

			if constexpr (std::is_integral_v<T> && std::is_integral_v<V>)
			{
				using CU = std::make_unsigned_t<C>;

				if constexpr (std::is_unsigned_v<T>)
					return v >= 0 && CU(v) <= CU(std::numeric_limits<T>::max());

				if constexpr (std::is_unsigned_v<V>)
					return CU(v) <= CU(std::numeric_limits<T>::max());

				// NOLINTNEXTLINE(misc-redundant-expression) False positive. TODO: Fixed? File bug?
				return C(v) <= C(std::numeric_limits<T>::max()) &&
				       C(v) >= C(std::numeric_limits<T>::lowest());
			}

			if constexpr (std::is_integral_v<T> && std::is_floating_point_v<V>)
				return float_to_integer(v) <= std::numeric_limits<T>::max() &&
				       float_to_integer(v) >= std::numeric_limits<T>::lowest();

			if constexpr (std::is_floating_point_v<T> && std::is_integral_v<V>)
				return true;

			if constexpr (std::is_floating_point_v<T> && std::is_floating_point_v<V>)
				return sizeof(T) >= sizeof(V) || (C(v) <= C(std::numeric_limits<T>::max()) &&
				                                  C(v) >= C(std::numeric_limits<T>::lowest()));

			return false;
		};

		if (!within_range())
			return std::nullopt;

		if constexpr (std::is_integral_v<T> && std::is_floating_point_v<V>)
			return static_cast<T>(float_to_integer(v));

		return static_cast<T>(v);
	}
}

#endif // RAYNI_LIB_MATH_NUMERIC_CAST_H
