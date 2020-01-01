// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/string.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <locale>
#include <string>

namespace
{
	const char LOCALE_WITH_COMMA_AS_DECIMAL_SEPARATOR[] = "sv_SE.UTF-8";

	class ScopedLocale
	{
	public:
		explicit ScopedLocale(const std::string &locale_name)
		{
			std::locale locale(locale_name);
			original_locale_ = std::locale::global(locale);
		}

		~ScopedLocale()
		{
			std::locale::global(original_locale_);
		}

		ScopedLocale(const ScopedLocale &) = delete;
		ScopedLocale(ScopedLocale &&) = delete;
		ScopedLocale &operator=(const ScopedLocale &) = delete;
		ScopedLocale &operator=(ScopedLocale &&) = delete;

	private:
		std::locale original_locale_;
	};
}

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

	TEST(String, ToNumberFloat)
	{
		ScopedLocale locale(LOCALE_WITH_COMMA_AS_DECIMAL_SEPARATOR);

		EXPECT_FLOAT_EQ(1.0F, string_to_number<float>("1").value_or(0));
		EXPECT_FLOAT_EQ(1.0F, string_to_number<float>("1.0").value_or(0));
		EXPECT_FLOAT_EQ(1.0F, string_to_number<float>(" 1").value_or(0));
		EXPECT_FLOAT_EQ(1.0F, string_to_number<float>("+1").value_or(0));
		EXPECT_FLOAT_EQ(-1.0F, string_to_number<float>("-1").value_or(0));
		EXPECT_FLOAT_EQ(0.1F, string_to_number<float>(".1").value_or(0));
		EXPECT_FLOAT_EQ(1.0F, string_to_number<float>("1.").value_or(0));
		EXPECT_FLOAT_EQ(12.34F, string_to_number<float>("12.34").value_or(0));
		EXPECT_FLOAT_EQ(-12.34F, string_to_number<float>("-12.34").value_or(0));
		EXPECT_FLOAT_EQ(1230.0F, string_to_number<float>("0.123e4").value_or(0));
		EXPECT_FLOAT_EQ(0.0123F, string_to_number<float>("123e-4").value_or(0));
		EXPECT_FLOAT_EQ(1230000.0F, string_to_number<float>("123e+4").value_or(0));

		EXPECT_FALSE(string_to_number<float>("").has_value());
		EXPECT_FALSE(string_to_number<float>(" ").has_value());
		EXPECT_FALSE(string_to_number<float>("1,0").has_value());
		EXPECT_FALSE(string_to_number<float>(",1").has_value());
		EXPECT_FALSE(string_to_number<float>("1,").has_value());
		EXPECT_FALSE(string_to_number<float>("").has_value());
		EXPECT_FALSE(string_to_number<float>("a").has_value());
		EXPECT_FALSE(string_to_number<float>("1a").has_value());
		EXPECT_FALSE(string_to_number<float>("1 ").has_value());
		EXPECT_FALSE(string_to_number<float>("a1").has_value());
		EXPECT_FALSE(string_to_number<float>("1e").has_value());
		EXPECT_FALSE(string_to_number<float>("+").has_value());
		EXPECT_FALSE(string_to_number<float>("-").has_value());
	}

	TEST(String, ToNumberDouble)
	{
		ScopedLocale locale(LOCALE_WITH_COMMA_AS_DECIMAL_SEPARATOR);

		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>("1").value_or(0));
		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>("1.0").value_or(0));
		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>(" 1").value_or(0));
		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>("+1").value_or(0));
		EXPECT_DOUBLE_EQ(-1.0, string_to_number<double>("-1").value_or(0));
		EXPECT_DOUBLE_EQ(0.1, string_to_number<double>(".1").value_or(0));
		EXPECT_DOUBLE_EQ(1.0, string_to_number<double>("1.").value_or(0));
		EXPECT_DOUBLE_EQ(12.34, string_to_number<double>("12.34").value_or(0));
		EXPECT_DOUBLE_EQ(-12.34, string_to_number<double>("-12.34").value_or(0));
		EXPECT_DOUBLE_EQ(1230.0, string_to_number<double>("0.123e4").value_or(0));
		EXPECT_DOUBLE_EQ(0.0123, string_to_number<double>("123e-4").value_or(0));
		EXPECT_DOUBLE_EQ(1230000.0, string_to_number<double>("123e+4").value_or(0));

		EXPECT_FALSE(string_to_number<double>("").has_value());
		EXPECT_FALSE(string_to_number<double>(" ").has_value());
		EXPECT_FALSE(string_to_number<double>("1,0").has_value());
		EXPECT_FALSE(string_to_number<double>(",1").has_value());
		EXPECT_FALSE(string_to_number<double>("1,").has_value());
		EXPECT_FALSE(string_to_number<double>("").has_value());
		EXPECT_FALSE(string_to_number<double>("a").has_value());
		EXPECT_FALSE(string_to_number<double>("1a").has_value());
		EXPECT_FALSE(string_to_number<double>("1 ").has_value());
		EXPECT_FALSE(string_to_number<double>("a1").has_value());
		EXPECT_FALSE(string_to_number<double>("1e").has_value());
		EXPECT_FALSE(string_to_number<double>("+").has_value());
		EXPECT_FALSE(string_to_number<double>("-").has_value());
	}

	TEST(String, ToNumberInt8)
	{
		EXPECT_EQ(1, string_to_number<std::int8_t>("1").value_or(0));
		EXPECT_EQ(-1, string_to_number<std::int8_t>("-1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::int8_t>("0").value_or(1));
		EXPECT_EQ(0, string_to_number<std::int8_t>("-0").value_or(1));
		EXPECT_EQ(123, string_to_number<std::int8_t>("123").value_or(0));
		EXPECT_EQ(127, string_to_number<std::int8_t>("127").value_or(0));
		EXPECT_EQ(-128, string_to_number<std::int8_t>("-128").value_or(0));

		EXPECT_FALSE(string_to_number<std::int8_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("128").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("-129").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::int8_t>("1.23e2").has_value());
	}

	TEST(String, ToNumberUInt8)
	{
		EXPECT_EQ(1, string_to_number<std::uint8_t>("1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::uint8_t>("0").value_or(1));
		EXPECT_EQ(123, string_to_number<std::uint8_t>("123").value_or(0));
		EXPECT_EQ(255, string_to_number<std::uint8_t>("255").value_or(0));

		EXPECT_FALSE(string_to_number<std::uint8_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("-1").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("-0").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("256").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::uint8_t>("1.23e2").has_value());
	}

	TEST(String, ToNumberInt16)
	{
		EXPECT_EQ(1, string_to_number<std::int16_t>("1").value_or(0));
		EXPECT_EQ(-1, string_to_number<std::int16_t>("-1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::int16_t>("0").value_or(1));
		EXPECT_EQ(0, string_to_number<std::int16_t>("-0").value_or(1));
		EXPECT_EQ(12345, string_to_number<std::int16_t>("12345").value_or(0));
		EXPECT_EQ(32767, string_to_number<std::int16_t>("32767").value_or(0));
		EXPECT_EQ(-32768, string_to_number<std::int16_t>("-32768").value_or(0));

		EXPECT_FALSE(string_to_number<std::int16_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("32768").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("-32769").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::int16_t>("1.23e2").has_value());
	}

	TEST(String, ToNumberUInt16)
	{
		EXPECT_EQ(1, string_to_number<std::uint16_t>("1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::uint16_t>("0").value_or(1));
		EXPECT_EQ(12345, string_to_number<std::uint16_t>("12345").value_or(0));
		EXPECT_EQ(65535, string_to_number<std::uint16_t>("65535").value_or(0));

		EXPECT_FALSE(string_to_number<std::uint16_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("-1").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("-0").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("65536").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::uint16_t>("1.23e2").has_value());
	}

	TEST(String, ToNumberInt32)
	{
		EXPECT_EQ(1, string_to_number<std::int32_t>("1").value_or(0));
		EXPECT_EQ(-1, string_to_number<std::int32_t>("-1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::int32_t>("0").value_or(1));
		EXPECT_EQ(0, string_to_number<std::int32_t>("-0").value_or(1));
		EXPECT_EQ(1234567890, string_to_number<std::int32_t>("1234567890").value_or(0));
		EXPECT_EQ(2147483647, string_to_number<std::int32_t>("2147483647").value_or(0));
		EXPECT_EQ(-2147483648, string_to_number<std::int32_t>("-2147483648").value_or(0));

		EXPECT_FALSE(string_to_number<std::int32_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("2147483648").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("-2147483649").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::int32_t>("1.23e2").has_value());
	}

	TEST(String, ToNumberUInt32)
	{
		EXPECT_EQ(1, string_to_number<std::uint32_t>("1").value_or(0));
		EXPECT_EQ(0, string_to_number<std::uint32_t>("0").value_or(1));
		EXPECT_EQ(1234567890, string_to_number<std::uint32_t>("1234567890").value_or(0));
		EXPECT_EQ(4294967295, string_to_number<std::uint32_t>("4294967295").value_or(0));

		EXPECT_FALSE(string_to_number<std::uint32_t>("").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>(" ").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>(" 1").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("1 ").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("+1").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("-1").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("+0").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("-0").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("1a").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("4294967296").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("1.2").has_value());
		EXPECT_FALSE(string_to_number<std::uint32_t>("1.23e2").has_value());
	}
}
