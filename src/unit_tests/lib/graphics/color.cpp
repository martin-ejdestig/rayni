/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/graphics/color.h"

#include <gtest/gtest.h>

#include <cmath>

#include "lib/containers/variant.h"

namespace Rayni
{
	namespace
	{
		testing::AssertionResult color_near(const char *c1_expr,
		                                    const char *c2_expr,
		                                    const Color &c1,
		                                    const Color &c2)
		{
			static constexpr real_t COMPONENT_MAX_DIFF = 1e-7;
			Color diff = c1 - c2;

			if (std::abs(diff.r()) > COMPONENT_MAX_DIFF || std::abs(diff.g()) > COMPONENT_MAX_DIFF ||
			    std::abs(diff.b()) > COMPONENT_MAX_DIFF)
			{
				return testing::AssertionFailure()
				       << c1_expr << " and " << c2_expr << " componentwise difference is (" << diff.r()
				       << ", " << diff.g() << ", " << diff.b() << ").";
			}

			return testing::AssertionSuccess();
		}
	}

	TEST(Color, Variant)
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

	TEST(Color, OperatorAddition)
	{
		Color c = Color(0.1, 0.2, 0.3) + Color(0.4, 0.5, 0.6);
		EXPECT_PRED_FORMAT2(color_near, Color(0.5, 0.7, 0.9), c);
	}

	TEST(Color, OperatorSubtraction)
	{
		Color c = Color(0.6, 0.5, 0.4) - Color(0.1, 0.2, 0.3);
		EXPECT_PRED_FORMAT2(color_near, Color(0.5, 0.3, 0.1), c);
	}

	TEST(Color, OperatorAdditionAssignment)
	{
		Color c(0.1, 0.2, 0.3);
		c += Color(0.4, 0.5, 0.6);
		EXPECT_PRED_FORMAT2(color_near, Color(0.5, 0.7, 0.9), c);
	}

	TEST(Color, OperatorMultiplication)
	{
		Color c = Color(0.4, 0.6, 0.8) * Color(0.5, 0.25, 0.75);
		EXPECT_PRED_FORMAT2(color_near, Color(0.2, 0.15, 0.6), c);
	}

	TEST(Color, OperatorMultiplicationAssignment)
	{
		Color c(0.4, 0.6, 0.8);
		c *= Color(0.5, 0.25, 0.75);
		EXPECT_PRED_FORMAT2(color_near, Color(0.2, 0.15, 0.6), c);
	}

	TEST(Color, OperatorsMultiplicationScalar)
	{
		Color c = Color(0.1, 0.2, 0.3) * real_t(2);
		EXPECT_PRED_FORMAT2(color_near, Color(0.2, 0.4, 0.6), c);
	}

	TEST(Color, OperatorMultiplicationScalarColor)
	{
		Color c = real_t(2) * Color(0.3, 0.1, 0.2);
		EXPECT_PRED_FORMAT2(color_near, Color(0.6, 0.2, 0.4), c);
	}

	TEST(Color, OperatorMultiplicationScalarAssignment)
	{
		Color c(0.2, 0.3, 0.1);
		c *= real_t(2);
		EXPECT_PRED_FORMAT2(color_near, Color(0.4, 0.6, 0.2), c);
	}

	TEST(Color, Clamp)
	{
		EXPECT_PRED_FORMAT2(color_near, Color::black(), Color(-0.1, -0.2, -0.3).clamp());
		EXPECT_PRED_FORMAT2(color_near, Color(0.3, 0.5, 0.7), Color(0.3, 0.5, 0.7).clamp());
		EXPECT_PRED_FORMAT2(color_near, Color::white(), Color(1.1, 1.2, 1.3).clamp());
	}

	TEST(Color, OKToOverflowWhenAddingAndTakeAverageForSupersampling)
	{
		Color c = Color(1, 0.5, 0.3) + Color(1, 0.5, 0.3) + Color(1, 0.5, 0.3) + Color(1, 0.5, 0.3);
		c *= 0.25;
		EXPECT_PRED_FORMAT2(color_near, Color(1, 0.5, 0.3), c);
	}
}
