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

#include "lib/math/enum.h"

#include <gtest/gtest.h>

#include <array>

namespace Rayni
{
	TEST(Enum, ToAndFromValue)
	{
		enum class Foo
		{
			BAR = 123,
			BAZ = 456
		};

		EXPECT_EQ(123, enum_to_value(Foo::BAR));
		EXPECT_EQ(456, enum_to_value(Foo::BAZ));

		static const std::array<Foo, 2> enum_values = {Foo::BAR, Foo::BAZ};
		EXPECT_EQ(Foo::BAR, enum_from_value(enum_values, 123).value());
		EXPECT_EQ(Foo::BAZ, enum_from_value(enum_values, 456).value());
		EXPECT_FALSE(enum_from_value(enum_values, 789).has_value());

		EXPECT_EQ(Foo::BAR, enum_from_value({Foo::BAR, Foo::BAZ}, 123).value());
		EXPECT_EQ(Foo::BAZ, enum_from_value({Foo::BAR, Foo::BAZ}, 456).value());
		EXPECT_FALSE(enum_from_value({Foo::BAR, Foo::BAZ}, 789).has_value());
	}
}
