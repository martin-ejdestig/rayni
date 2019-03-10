// This file is part of Rayni.
//
// Copyright (C) 2016-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/json_reader.h"

#include <gtest/gtest.h>

#include "lib/containers/variant.h"

namespace Rayni
{
	TEST(JSONReader, Null)
	{
		EXPECT_TRUE(JSONReader().read_string("null").is_none());
	}

	TEST(JSONReader, Bool)
	{
		EXPECT_EQ(true, JSONReader().read_string("true").as_bool());
		EXPECT_EQ(false, JSONReader().read_string("false").as_bool());
	}

	TEST(JSONReader, Number)
	{
		EXPECT_THROW(JSONReader().read_string("-"), JSONReader::Exception);

		EXPECT_NEAR(0, JSONReader().read_string("0").as_double(), 1e-100);
		EXPECT_NEAR(0, JSONReader().read_string("-0").as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("00"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-00"), JSONReader::Exception);

		EXPECT_NEAR(123, JSONReader().read_string("123").as_double(), 1e-100);
		EXPECT_NEAR(-123, JSONReader().read_string("-123").as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("0123"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-0123"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123a"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123a"), JSONReader::Exception);

		EXPECT_NEAR(1.23, JSONReader().read_string("1.23").as_double(), 1e-100);
		EXPECT_NEAR(-1.23, JSONReader().read_string("-1.23").as_double(), 1e-100);
		EXPECT_NEAR(12.3, JSONReader().read_string("12.3").as_double(), 1e-100);
		EXPECT_NEAR(-12.3, JSONReader().read_string("-12.3").as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("123."), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123."), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string(".123"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-.123"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("12,3"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-12,3"), JSONReader::Exception);

		EXPECT_NEAR(123e2, JSONReader().read_string("123e2").as_double(), 1e-100);
		EXPECT_NEAR(-123e2, JSONReader().read_string("-123e2").as_double(), 1e-100);
		EXPECT_NEAR(123e+2, JSONReader().read_string("123e+2").as_double(), 1e-100);
		EXPECT_NEAR(-123e+2, JSONReader().read_string("-123e+2").as_double(), 1e-100);
		EXPECT_NEAR(123e-2, JSONReader().read_string("123e-2").as_double(), 1e-100);
		EXPECT_NEAR(-123e-2, JSONReader().read_string("-123e-2").as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("123e"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123e."), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e."), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123e+"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e+"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123e-"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e-"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123E2"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123E2"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123e2.0"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e2.0"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("123e2,0"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("-123e2,0"), JSONReader::Exception);
	}

	TEST(JSONReader, String)
	{
		EXPECT_EQ("", JSONReader().read_string("\"\"").as_string());
		EXPECT_EQ("a", JSONReader().read_string("\"a\"").as_string());
		EXPECT_EQ("abc", JSONReader().read_string("\"abc\"").as_string());
		EXPECT_EQ("\b\t\n\f\r\"\\", JSONReader().read_string("\"\\b\\t\\n\\f\\r\\\"\\\\\"").as_string());

		EXPECT_THROW(JSONReader().read_string("\""), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("\"abc\n\""), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("\"\\\u001f\""), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("\"\\a\""), JSONReader::Exception);
	}

	TEST(JSONReader, Array)
	{
		EXPECT_EQ(0, JSONReader().read_string("[]").as_vector().size());

		Variant variant1 = JSONReader().read_string("[12]");
		EXPECT_EQ(1u, variant1.as_vector().size());
		EXPECT_NEAR(12, variant1.get(0).as_double(), 1e-100);

		Variant variant4 = JSONReader().read_string("[   true, 56,\"abc\" \n ,90     \t\n\n\n\t   ]");
		EXPECT_EQ(4u, variant4.as_vector().size());
		EXPECT_EQ(true, variant4.get(0).as_bool());
		EXPECT_NEAR(56, variant4.get(1).as_double(), 1e-100);
		EXPECT_EQ("abc", variant4.get(2).as_string());
		EXPECT_NEAR(90, variant4.get(3).as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("["), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[ "), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1,"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1,]"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1, ]"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1,,]"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1, ,]"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1, , ]"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("[1,\"]\""), JSONReader::Exception);
	}

	TEST(JSONReader, Object)
	{
		EXPECT_EQ(0, JSONReader().read_string("{}").as_map().size());

		Variant variant1 = JSONReader().read_string("{\"ab\":12}");
		EXPECT_EQ(1u, variant1.as_map().size());
		EXPECT_NEAR(12, variant1.get("ab").as_double(), 1e-100);

		Variant variant3 = JSONReader().read_string("{   \"a\": false, \n \"b\":\n  \"xyz\",\n\"c\":  3   } ");
		EXPECT_EQ(3u, variant3.as_map().size());
		EXPECT_EQ(false, variant3.get("a").as_bool());
		EXPECT_EQ("xyz", variant3.get("b").as_string());
		EXPECT_NEAR(3, variant3.get("c").as_double(), 1e-100);

		EXPECT_THROW(JSONReader().read_string("{"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{ "), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a: }"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\" }"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\": "), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\" 1}"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\": }"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\": 1,}"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\": 1 ,}"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("{\"a\": 1 , }"), JSONReader::Exception);

		EXPECT_THROW(JSONReader().read_string("{\"duplicate\": 1, \"duplicate\": 2}"), JSONReader::Exception);

		EXPECT_THROW(JSONReader().read_string("{12: \"key type not string\"}"), JSONReader::Exception);
	}

	TEST(JSONReader, Nested)
	{
		Variant root = JSONReader().read_string("{\"a\": [[1], [2, 3], {\"aa\": 4, \"ab\": {\"abc\": 5 }}]}");

		EXPECT_NEAR(1, root.get("a").get(0).get(0).as_double(), 1e-100);
		EXPECT_NEAR(2, root.get("a").get(1).get(0).as_double(), 1e-100);
		EXPECT_NEAR(3, root.get("a").get(1).get(1).as_double(), 1e-100);
		EXPECT_NEAR(4, root.get("a").get(2).get("aa").as_double(), 1e-100);
		EXPECT_NEAR(5, root.get("a").get(2).get("ab").get("abc").as_double(), 1e-100);
	}

	TEST(JSONReader, LeadingAndTrailingSpaceIgnored)
	{
		EXPECT_TRUE(JSONReader().read_string("true\n").as_bool());
		EXPECT_TRUE(JSONReader().read_string("\ntrue").as_bool());
		EXPECT_TRUE(JSONReader().read_string("\ntrue\n").as_bool());
		EXPECT_TRUE(JSONReader().read_string(" \n   \r\n \t true \n  \r\n \t ").as_bool());
		EXPECT_TRUE(JSONReader().read_string(" \n   \r\n \t true \n  \r\n \t ").as_bool());
	}

	TEST(JSONReader, TrailingGarbage)
	{
		EXPECT_THROW(JSONReader().read_string("true true"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("true \ntrue"), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("true \ntrue "), JSONReader::Exception);
		EXPECT_THROW(JSONReader().read_string("true \n true "), JSONReader::Exception);
	}
}
