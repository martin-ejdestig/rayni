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

#include "lib/math/numeric_cast.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>

// NOTE: Run tests in both release and debug mode after changing this file.
//       Behavior can be different due to -ffast-math.

static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559,
              "some tests currently assume IEEE 754 floating point arithmetic");

namespace Rayni
{
	TEST(NumericCast, Int8FromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));
		EXPECT_EQ(-128, cast(-128).value_or(0));
	}

	TEST(NumericCast, Int8FromUInt8)
	{
		auto cast = [](std::uint8_t in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));

		EXPECT_FALSE(cast(128).has_value());
		EXPECT_FALSE(cast(255).has_value());
	}

	TEST(NumericCast, Int8FromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));
		EXPECT_EQ(-128, cast(-128).value_or(0));

		EXPECT_FALSE(cast(128).has_value());
		EXPECT_FALSE(cast(-129).has_value());
	}

	TEST(NumericCast, Int8FromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));

		EXPECT_FALSE(cast(128).has_value());
		EXPECT_FALSE(cast(255).has_value());
	}

	TEST(NumericCast, Int8FromFloat)
	{
		auto cast = [](float in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0.0f).value_or(1));

		EXPECT_EQ(127, cast(127.0f).value_or(0));
		EXPECT_EQ(127, cast(127.4f).value_or(0));
		EXPECT_FALSE(cast(127.5f).has_value());
		EXPECT_FALSE(cast(128.0f).has_value());

		EXPECT_EQ(-128, cast(-128.0f).value_or(0));
		EXPECT_EQ(-128, cast(-128.4f).value_or(0));
		EXPECT_FALSE(cast(-128.5f).has_value());
		EXPECT_FALSE(cast(-129.0f).has_value());
	}

	TEST(NumericCast, Int8FromDouble)
	{
		auto cast = [](double in) { return numeric_cast<std::int8_t>(in); };

		EXPECT_EQ(0, cast(0.0).value_or(1));

		EXPECT_EQ(127, cast(127.0).value_or(0));
		EXPECT_EQ(127, cast(127.4).value_or(0));
		EXPECT_FALSE(cast(127.5).has_value());
		EXPECT_FALSE(cast(128.0).has_value());

		EXPECT_EQ(-128, cast(-128.0).value_or(0));
		EXPECT_EQ(-128, cast(-128.4).value_or(0));
		EXPECT_FALSE(cast(-128.5).has_value());
		EXPECT_FALSE(cast(-129.0).has_value());
	}

	TEST(NumericCast, UInt8FromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));

		EXPECT_FALSE(cast(-1).has_value());
		EXPECT_FALSE(cast(-128).has_value());
	}

	TEST(NumericCast, UInt8FromUInt8)
	{
		auto cast = [](std::uint8_t in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(255, cast(255).value_or(0));
	}

	TEST(NumericCast, UInt8FromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(255, cast(255).value_or(0));

		EXPECT_FALSE(cast(256).has_value());
		EXPECT_FALSE(cast(-1).has_value());
	}

	TEST(NumericCast, UInt8FromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(255, cast(255).value_or(0));

		EXPECT_FALSE(cast(256).has_value());
	}

	TEST(NumericCast, UInt8FromFloat)
	{
		auto cast = [](float in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0.0f).value_or(1));

		EXPECT_EQ(255, cast(255.0f).value_or(0));
		EXPECT_EQ(255, cast(255.4f).value_or(0));
		EXPECT_FALSE(cast(255.5f).has_value());
		EXPECT_FALSE(cast(256.0f).has_value());

		EXPECT_EQ(0, cast(-0.4f).value_or(0));
		EXPECT_FALSE(cast(-0.5f).has_value());
		EXPECT_FALSE(cast(-1.0f).has_value());
	}

	TEST(NumericCast, UInt8FromDouble)
	{
		auto cast = [](double in) { return numeric_cast<std::uint8_t>(in); };

		EXPECT_EQ(0, cast(0.0).value_or(1));

		EXPECT_EQ(255, cast(255.0).value_or(0));
		EXPECT_EQ(255, cast(255.4).value_or(0));
		EXPECT_FALSE(cast(255.5).has_value());
		EXPECT_FALSE(cast(256.0).has_value());

		EXPECT_EQ(0, cast(-0.4).value_or(0));
		EXPECT_FALSE(cast(-0.5).has_value());
		EXPECT_FALSE(cast(-1.0).has_value());
	}

	TEST(NumericCast, Int32FromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));
		EXPECT_EQ(-128, cast(-128).value_or(0));
	}

	TEST(NumericCast, Int32FromUInt8)
	{
		auto cast = [](std::uint8_t in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(255, cast(255).value_or(0));
	}

	TEST(NumericCast, Int32FromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(2147483647, cast(2147483647).value_or(0));
		EXPECT_EQ(-2147483648, cast(-2147483648).value_or(0));
	}

	TEST(NumericCast, Int32FromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(2147483647, cast(2147483647).value_or(0));

		EXPECT_FALSE(cast(2147483648).has_value());
		EXPECT_FALSE(cast(4294967295).has_value());
	}

	TEST(NumericCast, Int32FromFloat)
	{
		auto cast = [](float in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0.0f).value_or(1));

		EXPECT_EQ(2147483520, cast(2147483520.0f).value_or(0)); // 2^31 - 128
		EXPECT_EQ(2147483520, cast(2147483583.0f).value_or(0)); // 2^31 - 128 + 63
		EXPECT_FALSE(cast(2147483584.0f).has_value()); // 2^31 - 128 + 64
		EXPECT_FALSE(cast(2147483647.0f).has_value()); // 2^31 - 1

		EXPECT_EQ(-2147483648, cast(-2147483648.0f).value_or(0)); // - 2^31
		EXPECT_EQ(-2147483648, cast(-2147483776.0f).value_or(0)); // - 2^31 - 128
		EXPECT_FALSE(cast(-2147483777.0f).has_value()); // - 2^31 - 129
		EXPECT_FALSE(cast(-2147483904.0f).has_value()); // - 2^31 - 256
	}

	TEST(NumericCast, Int32FromDouble)
	{
		auto cast = [](double in) { return numeric_cast<std::int32_t>(in); };

		EXPECT_EQ(0, cast(0.0).value_or(1));

		EXPECT_EQ(2147483647, cast(2147483647.0).value_or(0));
		EXPECT_EQ(2147483647, cast(2147483647.4).value_or(0));
		EXPECT_FALSE(cast(2147483647.5).has_value());
		EXPECT_FALSE(cast(2147483648.0).has_value());

		EXPECT_EQ(-2147483648, cast(-2147483648.0).value_or(0));
		EXPECT_EQ(-2147483648, cast(-2147483648.4).value_or(0));
		EXPECT_FALSE(cast(-2147483648.5).has_value());
		EXPECT_FALSE(cast(-2147483649.0).has_value());
	}

	TEST(NumericCast, UInt32FromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));

		EXPECT_FALSE(cast(-1).has_value());
		EXPECT_FALSE(cast(-128).has_value());
	}

	TEST(NumericCast, UInt32FromUInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(127, cast(127).value_or(0));

		EXPECT_FALSE(cast(-128).has_value());
	}

	TEST(NumericCast, UInt32FromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(2147483647, cast(2147483647).value_or(0));

		EXPECT_FALSE(cast(-1).has_value());
		EXPECT_FALSE(cast(-2147483648).has_value());
	}

	TEST(NumericCast, UInt32FromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0).value_or(1));
		EXPECT_EQ(4294967295, cast(4294967295).value_or(0));
	}

	TEST(NumericCast, UInt32FromFloat)
	{
		auto cast = [](float in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0.0f).value_or(1));

		EXPECT_EQ(4294967040, cast(4294967040.0f).value_or(0)); // 2^32 - 256
		EXPECT_EQ(4294967040, cast(4294967167.0f).value_or(0)); // 2^32 - 256 + 127
		EXPECT_FALSE(cast(4294967168.0f).has_value()); // 2^32 - 256 + 128
		EXPECT_FALSE(cast(4294967295.0f).has_value()); // 2^32 - 1

		EXPECT_FALSE(cast(4294967295.5f).has_value());
		EXPECT_FALSE(cast(4294967296.0f).has_value());

		EXPECT_EQ(0, cast(-0.4f).value_or(0));
		EXPECT_FALSE(cast(-0.5f).has_value());
		EXPECT_FALSE(cast(-1.0f).has_value());
	}

	TEST(NumericCast, UInt32FromDouble)
	{
		auto cast = [](double in) { return numeric_cast<std::uint32_t>(in); };

		EXPECT_EQ(0, cast(0.0).value_or(1));

		EXPECT_EQ(4294967295, cast(4294967295.0).value_or(0));
		EXPECT_EQ(4294967295, cast(4294967295.4).value_or(0));
		EXPECT_FALSE(cast(4294967295.5).has_value());
		EXPECT_FALSE(cast(4294967296.0).has_value());

		EXPECT_EQ(0, cast(-0.4).value_or(0));
		EXPECT_FALSE(cast(-0.5).has_value());
		EXPECT_FALSE(cast(-1.0).has_value());
	}

	TEST(NumericCast, FloatFromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0).value_or(1.0f), 1e-100);
		EXPECT_NEAR(127.0f, cast(127).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-128.0f, cast(-128).value_or(0.0f), 1e-100);
	}

	TEST(NumericCast, FloatFromUInt8)
	{
		auto cast = [](std::uint8_t in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0).value_or(1.0f), 1e-100);
		EXPECT_NEAR(255.0f, cast(255).value_or(0.0f), 1e-100);
	}

	TEST(NumericCast, FloatFromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0).value_or(1.0f), 1e-100);

		EXPECT_NEAR(2147483520.0f, cast(2147483520).value_or(0.0f), 1e-100); // 2^31 - 128
		EXPECT_NEAR(2147483520.0f, cast(2147483583).value_or(0.0f), 1e-100); // 2^32 - 128 + 63
		EXPECT_NEAR(2147483648.0f, cast(2147483584).value_or(0.0f), 1e-100); // 2^32 - 128 + 64
		EXPECT_NEAR(2147483648.0f, cast(2147483647).value_or(0.0f), 1e-100); // 2^31 - 1

		EXPECT_NEAR(-2147483648.0f, cast(-2147483648).value_or(0.0f), 1e-100); // - 2^31
		EXPECT_NEAR(-2147483648.0f, cast(-2147483584).value_or(0.0f), 1e-100); // - 2^31 + 64
		EXPECT_NEAR(-2147483520.0f, cast(-2147483583).value_or(0.0f), 1e-100); // - 2^31 + 65
		EXPECT_NEAR(-2147483520.0f, cast(-2147483519).value_or(0.0f), 1e-100); // - 2^31 + 128
	}

	TEST(NumericCast, FloatFromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0).value_or(1.0f), 1e-100);

		EXPECT_NEAR(4294967040.0f, cast(4294967040).value_or(0.0f), 1e-100); // 2^32 - 256
		EXPECT_NEAR(4294967040.0f, cast(4294967167).value_or(0.0f), 1e-100); // 2^32 - 256 + 127
		EXPECT_NEAR(4294967296.0f, cast(4294967168).value_or(0.0f), 1e-100); // 2^32 - 256 + 128
		EXPECT_NEAR(4294967296.0f, cast(4294967295).value_or(0.0f), 1e-100); // 2^32 - 1
	}

	TEST(NumericCast, FloatFromFloat)
	{
		auto cast = [](float in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0.0f).value_or(1.0f), 1e-100);

		EXPECT_NEAR(12345.6789f, cast(12345.6789f).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-12345.6789f, cast(-12345.6789f).value_or(0.0f), 1e-100);

		EXPECT_NEAR(3.4e38f, cast(3.4e38f).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-3.4e38f, cast(-3.4e38f).value_or(0.0f), 1e-100);

		EXPECT_NEAR(1.17e-38f, cast(1.17e-38f).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-1.17e-38f, cast(-1.17e-38f).value_or(0.0f), 1e-100);
	}

	TEST(NumericCast, FloatFromDouble)
	{
		auto cast = [](double in) { return numeric_cast<float>(in); };

		EXPECT_NEAR(0.0f, cast(0.0).value_or(1.0f), 1e-100);

		EXPECT_NEAR(12345.6789f, cast(12345.6789).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-12345.6789f, cast(-12345.6789).value_or(0.0f), 1e-100);

		EXPECT_NEAR(3.4e38f, cast(3.4e38).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-3.4e38f, cast(-3.4e38).value_or(0.0f), 1e-100);
		EXPECT_FALSE(cast(3.5e38).has_value());
		EXPECT_FALSE(cast(-3.5e38).has_value());

		EXPECT_NEAR(1.17e-38f, cast(1.17e-38).value_or(0.0f), 1e-100);
		EXPECT_NEAR(-1.17e-38f, cast(-1.17e-38).value_or(0.0f), 1e-100);
	}

	TEST(NumericCast, DoubleFromInt8)
	{
		auto cast = [](std::int8_t in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0).value_or(1.0), 1e-100);
		EXPECT_NEAR(127.0, cast(127).value_or(0.0), 1e-100);
		EXPECT_NEAR(-128.0, cast(-128).value_or(0.0), 1e-100);
	}

	TEST(NumericCast, DoubleFromUInt8)
	{
		auto cast = [](std::uint8_t in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0).value_or(1.0), 1e-100);
		EXPECT_NEAR(255.0, cast(255).value_or(0.0), 1e-100);
	}

	TEST(NumericCast, DoubleFromInt32)
	{
		auto cast = [](std::int32_t in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0).value_or(1.0), 1e-100);
		EXPECT_NEAR(2147483647.0, cast(2147483647).value_or(0.0), 1e-100);
		EXPECT_NEAR(-2147483648.0, cast(-2147483648).value_or(0.0), 1e-100);
	}

	TEST(NumericCast, DoubleFromUInt32)
	{
		auto cast = [](std::uint32_t in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0).value_or(1.0), 1e-100);
		EXPECT_NEAR(4294967295.0, cast(4294967295).value_or(0.0), 1e-100);
	}

	TEST(NumericCast, DoubleFromFloat)
	{
		auto cast = [](float in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0.0f).value_or(1.0), 1e-100);

		EXPECT_NEAR(12345.6789, cast(12345.6789f).value_or(0.0), 1e-3);
		EXPECT_NEAR(-12345.6789, cast(-12345.6789f).value_or(0.0), 1e-3);

		EXPECT_NEAR(3.4e38, cast(3.4e38f).value_or(0.0), 1e31);
		EXPECT_NEAR(-3.4e38, cast(-3.4e38f).value_or(0.0), 1e31);

		EXPECT_NEAR(1.17e-38, cast(1.17e-38f).value_or(0.0), 1e-47);
		EXPECT_NEAR(-1.17e-38, cast(-1.17e-38f).value_or(0.0), 1e-47);
	}

	TEST(NumericCast, DoubleFromDouble)
	{
		auto cast = [](double in) { return numeric_cast<double>(in); };

		EXPECT_NEAR(0.0, cast(0.0).value_or(1.0), 1e-100);

		EXPECT_NEAR(12345.6789, cast(12345.6789).value_or(0.0), 1e-100);
		EXPECT_NEAR(-12345.6789, cast(-12345.6789).value_or(0.0), 1e-100);

		EXPECT_NEAR(1.79e308, cast(1.79e308).value_or(0.0), 1e-100);
		EXPECT_NEAR(-1.79e308, cast(-1.79e308).value_or(0.0), 1e-100);

		EXPECT_NEAR(2.22e-308, cast(2.22e-308).value_or(0.0), 1e-100);
		EXPECT_NEAR(-2.22e-308, cast(-2.22e-308).value_or(0.0), 1e-100);
	}

	TEST(NumericCast, FromFloatPlusMinusZero)
	{
		EXPECT_EQ(0, numeric_cast<std::int8_t>(+0.0f).value_or(1));
		EXPECT_EQ(0, numeric_cast<std::int8_t>(-0.0f).value_or(1));

		EXPECT_NEAR(0.0f, numeric_cast<float>(+0.0f).value_or(1.0f), 1e-100);
		EXPECT_NEAR(0.0f, numeric_cast<float>(-0.0f).value_or(1.0f), 1e-100);

		EXPECT_NEAR(0.0, numeric_cast<double>(+0.0f).value_or(1.0), 1e-100);
		EXPECT_NEAR(0.0, numeric_cast<double>(-0.0f).value_or(1.0), 1e-100);
	}

	TEST(NumericCast, FromDoublePlusMinusZero)
	{
		EXPECT_EQ(0, numeric_cast<std::int8_t>(+0.0).value_or(1));
		EXPECT_EQ(0, numeric_cast<std::int8_t>(-0.0).value_or(1));

		EXPECT_NEAR(0.0f, numeric_cast<float>(+0.0).value_or(1.0f), 1e-100);
		EXPECT_NEAR(0.0f, numeric_cast<float>(-0.0).value_or(1.0f), 1e-100);

		EXPECT_NEAR(0.0, numeric_cast<double>(+0.0).value_or(1.0), 1e-100);
		EXPECT_NEAR(0.0, numeric_cast<double>(-0.0).value_or(1.0), 1e-100);
	}

	TEST(NumericCast, FromFloatInfinity)
	{
		const float infinity = std::numeric_limits<float>::infinity();

		EXPECT_FALSE(numeric_cast<int>(infinity).has_value());
		EXPECT_FALSE(numeric_cast<int>(-infinity).has_value());

		EXPECT_EQ(infinity, numeric_cast<float>(infinity).value_or(0.0f));
		EXPECT_EQ(-infinity, numeric_cast<float>(-infinity).value_or(0.0f));

		const double double_infinity = std::numeric_limits<double>::infinity();
		EXPECT_EQ(double_infinity, numeric_cast<double>(infinity).value_or(0.0));
		EXPECT_EQ(-double_infinity, numeric_cast<double>(-infinity).value_or(0.0));
	}

	TEST(NumericCast, FromDoubleInfinity)
	{
		const double infinity = std::numeric_limits<double>::infinity();

		EXPECT_FALSE(numeric_cast<int>(infinity).has_value());
		EXPECT_FALSE(numeric_cast<int>(-infinity).has_value());

		EXPECT_FALSE(numeric_cast<float>(infinity).has_value());
		EXPECT_FALSE(numeric_cast<float>(-infinity).has_value());

		EXPECT_EQ(infinity, numeric_cast<double>(infinity).value_or(0.0));
		EXPECT_EQ(-infinity, numeric_cast<double>(-infinity).value_or(0.0));
	}

	TEST(NumericCast, FromFloatQuietNaN)
	{
		static_assert(std::numeric_limits<float>::has_quiet_NaN);

		const float quiet_nan = std::numeric_limits<float>::quiet_NaN();

		// TODO: Currently does not work consistently with -ffast-math in GCC and Clang.
		// EXPECT_FALSE(numeric_cast<int>(quiet_nan).has_value());

		EXPECT_TRUE(numeric_cast<float>(quiet_nan).has_value());
		EXPECT_TRUE(numeric_cast<double>(quiet_nan).has_value());
	}

	TEST(NumericCast, FromDoubleQuietNaN)
	{
		static_assert(std::numeric_limits<double>::has_quiet_NaN);

		const double quiet_nan = std::numeric_limits<double>::quiet_NaN();

		// TODO: Currently does not work consistently with -ffast-math in GCC and Clang.
		// EXPECT_FALSE(numeric_cast<int>(quiet_nan).has_value());

		EXPECT_FALSE(numeric_cast<float>(quiet_nan).has_value()); // TODO: Different... OK? Change?
		EXPECT_TRUE(numeric_cast<double>(quiet_nan).has_value());
	}
}
