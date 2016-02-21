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

#ifndef RAYNI_LIB_CONTAINERS_FIXED_SIZE_STACK_H
#define RAYNI_LIB_CONTAINERS_FIXED_SIZE_STACK_H

#include <cassert>
#include <utility>

namespace Rayni
{
	template <typename Type, unsigned int MAX_SIZE>
	class FixedSizeStack
	{
	public:
		bool is_empty() const
		{
			return size == 0;
		}

		void push(const Type &&value)
		{
			assert(size < MAX_SIZE);
			array[size] = std::move(value);
			size++;
		}

		void pop()
		{
			assert(!is_empty());
			size--;
		}

		Type &top()
		{
			assert(!is_empty());
			return array[size - 1];
		}

		const Type &top() const
		{
			assert(!is_empty());
			return array[size - 1];
		}

	private:
		unsigned int size = 0;
		Type array[MAX_SIZE];
	};
}

#endif // RAYNI_LIB_CONTAINERS_FIXED_SIZE_STACK_H
