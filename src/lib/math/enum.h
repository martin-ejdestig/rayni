/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_MATH_ENUM_H_
#define _RAYNI_LIB_MATH_ENUM_H_

#include <algorithm>
#include <experimental/optional>
#include <type_traits>

namespace Rayni
{
	template <typename E, typename U = typename std::underlying_type<E>::type>
	static U enum_to_value(E enum_value)
	{
		return static_cast<U>(enum_value);
	}

	template <typename EnumValues,
	          typename E = typename EnumValues::value_type,
	          typename U = typename std::underlying_type<E>::type>
	static std::experimental::optional<E> enum_from_value(const EnumValues &enum_values, U value)
	{
		auto iterator = std::find_if(enum_values.cbegin(),
		                             enum_values.cend(),
		                             [&](const E &enum_value)
		                             {
			                             return value == static_cast<U>(enum_value);
			                     });

		return iterator == enum_values.cend() ? std::experimental::nullopt :
		                                        std::experimental::make_optional(*iterator);
	}
}

#endif // _RAYNI_LIB_MATH_ENUM_H_
