/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_MATH_HASH_H_
#define _RAYNI_LIB_MATH_HASH_H_

#include <cstddef>
#include <functional>

namespace Rayni
{
	static inline std::size_t hash_combine(std::size_t hash1, std::size_t hash2)
	{
		return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
	}

	template <typename T1, typename T2>
	static inline std::size_t hash_combine_for(const T1 &value1, const T2 &value2)
	{
		return hash_combine(std::hash<T1>()(value1), std::hash<T2>()(value2));
	}

	template <typename T1, typename T2, typename... Args>
	static inline std::size_t hash_combine_for(const T1 &value1, const T2 &value2, const Args &... args)
	{
		return hash_combine(std::hash<T1>()(value1), hash_combine_for(value2, args...));
	}
}

#endif // _RAYNI_LIB_MATH_HASH_H_
