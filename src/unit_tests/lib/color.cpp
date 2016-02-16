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

#include <cmath>

#include "lib/color.h"
#include "lib/containers/variant.h"

namespace Rayni
{
	class ColorTest : public testing::Test
	{
	protected:
		static testing::AssertionResult color_near(const char *c1_expr,
		                                           const char *c2_expr,
		                                           const Color &c1,
		                                           const Color &c2)
		{
			static const real_t COMPONENT_MAX_DIFF = 0.001;

			if (std::abs(c1.r() - c2.r()) > COMPONENT_MAX_DIFF ||
			    std::abs(c1.g() - c2.g()) > COMPONENT_MAX_DIFF ||
			    std::abs(c1.b() - c2.b()) > COMPONENT_MAX_DIFF)
			{
				return testing::AssertionFailure()
				       << c1_expr << " and " << c2_expr << " componentwise difference is ("
				       << c1.r() - c2.r() << ", " << c1.g() - c2.g() << ", " << c1.b() - c2.b() << ").";
			}

			return testing::AssertionSuccess();
		}
	};

	TEST_F(ColorTest, Variant)
	{
		EXPECT_PRED_FORMAT2(color_near, Color::black(), Variant("black").to<Color>());
		EXPECT_PRED_FORMAT2(color_near, Color::white(), Variant("white").to<Color>());
		EXPECT_PRED_FORMAT2(color_near, Color::red(), Variant("red").to<Color>());
		EXPECT_PRED_FORMAT2(color_near, Color::yellow(), Variant("yellow").to<Color>());
		EXPECT_PRED_FORMAT2(color_near, Color::green(), Variant("green").to<Color>());
		EXPECT_PRED_FORMAT2(color_near, Color::blue(), Variant("blue").to<Color>());

		EXPECT_PRED_FORMAT2(color_near, Color(0.1, 0.2, 0.3), Variant::vector(0.1, 0.2, 0.3).to<Color>());

		EXPECT_THROW(Variant().to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant::vector(0).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant(true).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant(0).to<Color>(), Variant::Exception);
		EXPECT_THROW(Variant("").to<Color>(), Variant::Exception);
	}

	TEST_F(ColorTest, Clamp)
	{
		EXPECT_PRED_FORMAT2(color_near, Color::black(), Color(-0.1, -0.2, -0.3).clamp());
		EXPECT_PRED_FORMAT2(color_near, Color(0.3, 0.5, 0.7), Color(0.3, 0.5, 0.7).clamp());
		EXPECT_PRED_FORMAT2(color_near, Color::white(), Color(1.1, 1.2, 1.3).clamp());
	}
}
