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

#include <gtest/gtest.h>

#include "lib/color.h"
#include "lib/containers/variant.h"

namespace Rayni
{
	TEST(ColorTest, Variant)
	{
		EXPECT_EQ(Color::black(), Variant("black").to<Color>());
		EXPECT_EQ(Color::white(), Variant("white").to<Color>());
		EXPECT_EQ(Color::red(), Variant("red").to<Color>());
		EXPECT_EQ(Color::yellow(), Variant("yellow").to<Color>());
		EXPECT_EQ(Color::green(), Variant("green").to<Color>());
		EXPECT_EQ(Color::blue(), Variant("blue").to<Color>());

		EXPECT_EQ(Color(0.1, 0.2, 0.3), Variant::vector(0.1, 0.2, 0.3).to<Color>());

		EXPECT_THROW(Variant().to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant::vector(0).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant(true).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant(0).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant("").to<Color>(), Variant::Exception);
	}

	TEST(ColorTest, Clamp)
	{
		EXPECT_EQ(Color::black(), Color(-0.1, -0.2, -0.3).clamp());
		EXPECT_EQ(Color(0.3, 0.5, 0.7), Color(0.3, 0.5, 0.7).clamp());
		EXPECT_EQ(Color::white(), Color(1.1, 1.2, 1.3).clamp());
	}
}
