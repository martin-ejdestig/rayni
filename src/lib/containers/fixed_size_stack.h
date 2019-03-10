// This file is part of Rayni.
//
// Copyright (C) 2015-2019 Martin Ejdestig <marejde@gmail.com>
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
			return size_ == 0;
		}

		void push(Type &&value)
		{
			assert(size_ < MAX_SIZE);
			array_[size_] = std::move(value);
			size_++;
		}

		void pop()
		{
			assert(!is_empty());
			size_--;
		}

		Type &top()
		{
			assert(!is_empty());
			return array_[size_ - 1];
		}

		const Type &top() const
		{
			assert(!is_empty());
			return array_[size_ - 1];
		}

	private:
		unsigned int size_ = 0;
		Type array_[MAX_SIZE];
	};
}

#endif // RAYNI_LIB_CONTAINERS_FIXED_SIZE_STACK_H
