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

#include "lib/string/string.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>

namespace Rayni
{
	TEST(String, Center)
	{
		EXPECT_EQ("", string_center(0, ""));
		EXPECT_EQ("a", string_center(0, "a"));
		EXPECT_EQ("ab", string_center(0, "ab"));
		EXPECT_EQ("abc", string_center(0, "abc"));

		EXPECT_EQ(" ", string_center(1, ""));
		EXPECT_EQ("a", string_center(1, "a"));
		EXPECT_EQ("ab", string_center(1, "ab"));
		EXPECT_EQ("abc", string_center(1, "abc"));

		EXPECT_EQ("  ", string_center(2, ""));
		EXPECT_EQ("a ", string_center(2, "a"));
		EXPECT_EQ("ab", string_center(2, "ab"));
		EXPECT_EQ("abc", string_center(2, "abc"));

		EXPECT_EQ("   ", string_center(3, ""));
		EXPECT_EQ(" a ", string_center(3, "a"));
		EXPECT_EQ("ab ", string_center(3, "ab"));
		EXPECT_EQ("abc", string_center(3, "abc"));

		EXPECT_EQ("    ", string_center(4, ""));
		EXPECT_EQ(" a  ", string_center(4, "a"));
		EXPECT_EQ(" ab ", string_center(4, "ab"));
		EXPECT_EQ("abc ", string_center(4, "abc"));

		EXPECT_EQ("     ", string_center(5, ""));
		EXPECT_EQ("  a  ", string_center(5, "a"));
		EXPECT_EQ(" ab  ", string_center(5, "ab"));
		EXPECT_EQ(" abc ", string_center(5, "abc"));
	}

	TEST(String, RightAlign)
	{
		EXPECT_EQ("", string_right_align(0, ""));
		EXPECT_EQ("a", string_right_align(0, "a"));
		EXPECT_EQ("ab", string_right_align(0, "ab"));
		EXPECT_EQ("abc", string_right_align(0, "abc"));

		EXPECT_EQ(" ", string_right_align(1, ""));
		EXPECT_EQ("a", string_right_align(1, "a"));
		EXPECT_EQ("ab", string_right_align(1, "ab"));
		EXPECT_EQ("abc", string_right_align(1, "abc"));

		EXPECT_EQ("  ", string_right_align(2, ""));
		EXPECT_EQ(" a", string_right_align(2, "a"));
		EXPECT_EQ("ab", string_right_align(2, "ab"));
		EXPECT_EQ("abc", string_right_align(2, "abc"));

		EXPECT_EQ("   ", string_right_align(3, ""));
		EXPECT_EQ("  a", string_right_align(3, "a"));
		EXPECT_EQ(" ab", string_right_align(3, "ab"));
		EXPECT_EQ("abc", string_right_align(3, "abc"));

		EXPECT_EQ("    ", string_right_align(4, ""));
		EXPECT_EQ("   a", string_right_align(4, "a"));
		EXPECT_EQ("  ab", string_right_align(4, "ab"));
		EXPECT_EQ(" abc", string_right_align(4, "abc"));
	}

	TEST(String, ToLower)
	{
		EXPECT_EQ("abc_def123ghi#jkl", string_to_lower("abc_DEF123ghi#JKL"));
	}

	TEST(String, ToFloat)
	{
		EXPECT_FLOAT_EQ(1.0f, string_to_float("1").value());
		EXPECT_FLOAT_EQ(1.0f, string_to_float("1.0").value());
		EXPECT_FLOAT_EQ(1.0f, string_to_float(" 1").value());
		EXPECT_FLOAT_EQ(1.0f, string_to_float("+1").value());
		EXPECT_FLOAT_EQ(-1.0f, string_to_float("-1").value());
		EXPECT_FLOAT_EQ(0.1f, string_to_float(".1").value());
		EXPECT_FLOAT_EQ(1.0f, string_to_float("1.").value());
		EXPECT_FLOAT_EQ(12.34f, string_to_float("12.34").value());
		EXPECT_FLOAT_EQ(-12.34f, string_to_float("-12.34").value());
		EXPECT_FLOAT_EQ(1230.0f, string_to_float("0.123e4").value());
		EXPECT_FLOAT_EQ(0.0123f, string_to_float("123e-4").value());
		EXPECT_FLOAT_EQ(1230000.0f, string_to_float("123e+4").value());

		EXPECT_FALSE(string_to_float(""));
		EXPECT_FALSE(string_to_float(" "));
		EXPECT_FALSE(string_to_float("1,0"));
		EXPECT_FALSE(string_to_float(",1"));
		EXPECT_FALSE(string_to_float("1,"));
		EXPECT_FALSE(string_to_float(""));
		EXPECT_FALSE(string_to_float("a"));
		EXPECT_FALSE(string_to_float("1a"));
		EXPECT_FALSE(string_to_float("1 "));
		EXPECT_FALSE(string_to_float("a1"));
		EXPECT_FALSE(string_to_float("1e"));
		EXPECT_FALSE(string_to_float("+"));
		EXPECT_FALSE(string_to_float("-"));
	}

	TEST(String, ToDouble)
	{
		EXPECT_DOUBLE_EQ(1.0, string_to_double("1").value());
		EXPECT_DOUBLE_EQ(1.0, string_to_double("1.0").value());
		EXPECT_DOUBLE_EQ(1.0, string_to_double(" 1").value());
		EXPECT_DOUBLE_EQ(1.0, string_to_double("+1").value());
		EXPECT_DOUBLE_EQ(-1.0, string_to_double("-1").value());
		EXPECT_DOUBLE_EQ(0.1, string_to_double(".1").value());
		EXPECT_DOUBLE_EQ(1.0, string_to_double("1.").value());
		EXPECT_DOUBLE_EQ(12.34, string_to_double("12.34").value());
		EXPECT_DOUBLE_EQ(-12.34, string_to_double("-12.34").value());
		EXPECT_DOUBLE_EQ(1230.0, string_to_double("0.123e4").value());
		EXPECT_DOUBLE_EQ(0.0123, string_to_double("123e-4").value());
		EXPECT_DOUBLE_EQ(1230000.0, string_to_double("123e+4").value());

		EXPECT_FALSE(string_to_double(""));
		EXPECT_FALSE(string_to_double(" "));
		EXPECT_FALSE(string_to_double("1,0"));
		EXPECT_FALSE(string_to_double(",1"));
		EXPECT_FALSE(string_to_double("1,"));
		EXPECT_FALSE(string_to_double(""));
		EXPECT_FALSE(string_to_double("a"));
		EXPECT_FALSE(string_to_double("1a"));
		EXPECT_FALSE(string_to_double("1 "));
		EXPECT_FALSE(string_to_double("a1"));
		EXPECT_FALSE(string_to_double("1e"));
		EXPECT_FALSE(string_to_double("+"));
		EXPECT_FALSE(string_to_double("-"));
	}

	TEST(String, ToLong)
	{
		EXPECT_EQ(123, string_to_long("123").value());
		EXPECT_EQ(123, string_to_long(" 123").value());
		EXPECT_EQ(123, string_to_long("+123").value());
		EXPECT_EQ(-123, string_to_long("-123").value());
		EXPECT_EQ(123, string_to_long("0123").value());

		EXPECT_EQ(0, string_to_long("0").value());
		EXPECT_EQ(0, string_to_long("+0").value());
		EXPECT_EQ(0, string_to_long("-0").value());

		EXPECT_FALSE(string_to_long(""));
		EXPECT_FALSE(string_to_long(" "));
		EXPECT_FALSE(string_to_long("123 "));
		EXPECT_FALSE(string_to_long("123a"));
		EXPECT_FALSE(string_to_long("12.3"));
		EXPECT_FALSE(string_to_long("1.23e4"));
	}

	TEST(String, ToLongOutOfRange)
	{
		const long max = std::numeric_limits<long>::max();
		const long min = std::numeric_limits<long>::min();
		const std::string max_str = std::to_string(max);
		const std::string min_str = std::to_string(min);

		EXPECT_EQ(max, string_to_long(max_str).value());
		EXPECT_EQ(min, string_to_long(min_str).value());

		EXPECT_FALSE(string_to_long(max_str + "0"));
		EXPECT_FALSE(string_to_long(min_str + "0"));
	}

	TEST(String, ToNumber)
	{
		// Only testing template specific parts here. Non-template functions used in
		// string_to_number() tested more extensively above.

		EXPECT_FLOAT_EQ(1.0f, string_to_number<float>("1.0").value());

		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>("1.0").value());

		EXPECT_EQ(127, string_to_number<std::int8_t>("127").value());
		EXPECT_EQ(-128, string_to_number<std::int8_t>("-128").value());
		EXPECT_FALSE(string_to_number<std::int8_t>("128"));
		EXPECT_FALSE(string_to_number<std::int8_t>("-129"));

		EXPECT_EQ(255, string_to_number<std::uint8_t>("255").value());
		EXPECT_EQ(0, string_to_number<std::uint8_t>("0").value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("256"));
		EXPECT_FALSE(string_to_number<std::uint8_t>("-1"));
	}
}
