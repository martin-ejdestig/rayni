/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/containers/fixed_size_stack.h"

#include <gtest/gtest.h>

namespace Rayni
{
	namespace
	{
		struct Element
		{
			int value1 = 0;
			int value2 = 0;
		};
	}

	TEST(FixedSizeStack, IsEmpty)
	{
		FixedSizeStack<Element, 2> stack;

		EXPECT_TRUE(stack.is_empty());
		stack.push({12, 34});
		EXPECT_FALSE(stack.is_empty());
		stack.push({56, 78});
		EXPECT_FALSE(stack.is_empty());
		stack.pop();
		EXPECT_FALSE(stack.is_empty());
		stack.pop();
		EXPECT_TRUE(stack.is_empty());
	}

	TEST(FixedSizeStack, Top)
	{
		FixedSizeStack<Element, 2> stack;

		stack.push({12, 34});
		EXPECT_EQ(12, stack.top().value1);
		EXPECT_EQ(34, stack.top().value2);
		stack.push({56, 78});
		EXPECT_EQ(56, stack.top().value1);
		EXPECT_EQ(78, stack.top().value2);
		stack.pop();
		EXPECT_EQ(12, stack.top().value1);
		EXPECT_EQ(34, stack.top().value2);
	}
}
