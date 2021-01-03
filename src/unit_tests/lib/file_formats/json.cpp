// This file is part of Rayni.
//
// Copyright (C) 2016-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/json.h"

#include <gtest/gtest.h>

#include "lib/containers/variant.h"

namespace Rayni
{
	TEST(JSON, Null)
	{
		EXPECT_TRUE(json_read_string("null").value_or(Variant(false)).is_none());
	}

	TEST(JSON, Bool)
	{
		EXPECT_EQ(true, json_read_string("true").value_or(Variant(false)).to_bool().value_or(false));
		EXPECT_EQ(false, json_read_string("false").value_or(Variant(true)).to_bool().value_or(false));
	}

	TEST(JSON, Number)
	{
		EXPECT_FALSE(json_read_string("-"));

		EXPECT_NEAR(0, json_read_string("0").value_or(Variant(1.0)).to_double().value_or(1.1), 1e-100);
		EXPECT_NEAR(0, json_read_string("-0").value_or(Variant(1.0)).to_double().value_or(1.1), 1e-100);

		EXPECT_FALSE(json_read_string("00"));
		EXPECT_FALSE(json_read_string("-00"));

		EXPECT_NEAR(123, json_read_string("123").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);
		EXPECT_NEAR(-123, json_read_string("-123").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);

		EXPECT_FALSE(json_read_string("0123"));
		EXPECT_FALSE(json_read_string("-0123"));
		EXPECT_FALSE(json_read_string("123a"));
		EXPECT_FALSE(json_read_string("-123a"));

		EXPECT_NEAR(1.23, json_read_string("1.23").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);
		EXPECT_NEAR(-1.23, json_read_string("-1.23").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);
		EXPECT_NEAR(12.3, json_read_string("12.3").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);
		EXPECT_NEAR(-12.3, json_read_string("-12.3").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);

		EXPECT_FALSE(json_read_string("123."));
		EXPECT_FALSE(json_read_string("-123."));
		EXPECT_FALSE(json_read_string(".123"));
		EXPECT_FALSE(json_read_string("-.123"));
		EXPECT_FALSE(json_read_string("12,3"));
		EXPECT_FALSE(json_read_string("-12,3"));

		EXPECT_NEAR(123e2, json_read_string("123e2").value_or(Variant(0.0)).to_double().value_or(1.0), 1e-100);
		EXPECT_NEAR(-123e2,
		            json_read_string("-123e2").value_or(Variant(0.0)).to_double().value_or(1.0),
		            1e-100);
		EXPECT_NEAR(123e+2,
		            json_read_string("123e+2").value_or(Variant(0.0)).to_double().value_or(1.0),
		            1e-100);
		EXPECT_NEAR(-123e+2,
		            json_read_string("-123e+2").value_or(Variant(0.0)).to_double().value_or(1.0),
		            1e-100);
		EXPECT_NEAR(123e-2,
		            json_read_string("123e-2").value_or(Variant(0.0)).to_double().value_or(1.0),
		            1e-100);
		EXPECT_NEAR(-123e-2,
		            json_read_string("-123e-2").value_or(Variant(0.0)).to_double().value_or(1.0),
		            1e-100);

		EXPECT_FALSE(json_read_string("123e"));
		EXPECT_FALSE(json_read_string("-123e"));
		EXPECT_FALSE(json_read_string("123e."));
		EXPECT_FALSE(json_read_string("-123e."));
		EXPECT_FALSE(json_read_string("123e+"));
		EXPECT_FALSE(json_read_string("-123e+"));
		EXPECT_FALSE(json_read_string("123e-"));
		EXPECT_FALSE(json_read_string("-123e-"));
		EXPECT_FALSE(json_read_string("123E2"));
		EXPECT_FALSE(json_read_string("-123E2"));
		EXPECT_FALSE(json_read_string("123e2.0"));
		EXPECT_FALSE(json_read_string("-123e2.0"));
		EXPECT_FALSE(json_read_string("123e2,0"));
		EXPECT_FALSE(json_read_string("-123e2,0"));
	}

	TEST(JSON, String)
	{
		EXPECT_EQ("", json_read_string("\"\"").value_or(Variant("1")).to_string().value_or("2"));
		EXPECT_EQ("a", json_read_string("\"a\"").value_or(Variant("1")).to_string().value_or("2"));
		EXPECT_EQ("abc", json_read_string("\"abc\"").value_or(Variant("1")).to_string().value_or("2"));
		EXPECT_EQ("\b\t\n\f\r\"\\",
		          json_read_string("\"\\b\\t\\n\\f\\r\\\"\\\\\"")
		                  .value_or(Variant("1"))
		                  .to_string()
		                  .value_or("2"));

		EXPECT_FALSE(json_read_string("\""));
		EXPECT_FALSE(json_read_string("\"abc\n\""));
		EXPECT_FALSE(json_read_string("\"\\\u001f\""));
		EXPECT_FALSE(json_read_string("\"\\a\""));
	}

	TEST(JSON, Array)
	{
		Variant variant0 = json_read_string("[]").value_or(Variant::vector(1));
		ASSERT_TRUE(variant0.is_vector());
		EXPECT_EQ(0, variant0.as_vector().size());

		Variant variant1 = json_read_string("[12]").value_or(Variant::vector());
		ASSERT_TRUE(variant1.is_vector());
		EXPECT_EQ(1U, variant1.as_vector().size());
		ASSERT_TRUE(variant1.as_vector()[0].is_double());
		EXPECT_NEAR(12, variant1.as_vector()[0].as_double(), 1e-100);

		Variant variant4 = json_read_string("[   true, 56,\"abc\" \n ,90        \t\n\n\n\t   ]")
		                           .value_or(Variant::vector());
		ASSERT_TRUE(variant4.is_vector());
		EXPECT_EQ(4U, variant4.as_vector().size());
		ASSERT_TRUE(variant4.get(0)->is_bool());
		EXPECT_EQ(true, variant4.get(0)->as_bool());
		ASSERT_TRUE(variant4.get(1)->is_double());
		EXPECT_NEAR(56, variant4.get(1)->as_double(), 1e-100);
		ASSERT_TRUE(variant4.get(2)->is_string());
		EXPECT_EQ("abc", variant4.get(2)->as_string());
		ASSERT_TRUE(variant4.get(3)->is_double());
		EXPECT_NEAR(90, variant4.get(3)->as_double(), 1e-100);

		EXPECT_FALSE(json_read_string("["));
		EXPECT_FALSE(json_read_string("[ "));
		EXPECT_FALSE(json_read_string("[1,"));
		EXPECT_FALSE(json_read_string("[1,]"));
		EXPECT_FALSE(json_read_string("[1, ]"));
		EXPECT_FALSE(json_read_string("[1,,]"));
		EXPECT_FALSE(json_read_string("[1, ,]"));
		EXPECT_FALSE(json_read_string("[1, , ]"));
		EXPECT_FALSE(json_read_string("[1,\"]\""));
	}

	TEST(JSON, Object)
	{
		Variant variant0 = json_read_string("{}").value_or(Variant::map("a", 0));
		ASSERT_TRUE(variant0.is_map());
		EXPECT_EQ(0, variant0.as_map().size());

		Variant variant1 = json_read_string("{\"ab\":12}").value_or(Variant::map());
		ASSERT_TRUE(variant1.is_map());
		EXPECT_EQ(1U, variant1.as_map().size());
		ASSERT_TRUE(variant1.has("ab"));
		ASSERT_TRUE(variant1.get("ab")->is_double());
		EXPECT_NEAR(12, variant1.get("ab")->as_double(), 1e-100);

		Variant variant3 = json_read_string("{   \"a\": false, \n \"b\":\n  \"xyz\",\n\"c\":  3   } ")
		                           .value_or(Variant::map());
		ASSERT_TRUE(variant3.is_map());
		EXPECT_EQ(3U, variant3.as_map().size());
		ASSERT_TRUE(variant3.has("a"));
		ASSERT_TRUE(variant3.get("a")->is_bool());
		EXPECT_EQ(false, variant3.get("a")->as_bool());
		ASSERT_TRUE(variant3.has("b"));
		ASSERT_TRUE(variant3.get("b")->is_string());
		EXPECT_EQ("xyz", variant3.get("b")->as_string());
		ASSERT_TRUE(variant3.has("c"));
		ASSERT_TRUE(variant3.get("c")->is_double());
		EXPECT_NEAR(3, variant3.get("c")->as_double(), 1e-100);

		EXPECT_FALSE(json_read_string("{"));
		EXPECT_FALSE(json_read_string("{ "));
		EXPECT_FALSE(json_read_string("{\"a: }"));
		EXPECT_FALSE(json_read_string("{\"a\" }"));
		EXPECT_FALSE(json_read_string("{\"a\": "));
		EXPECT_FALSE(json_read_string("{\"a\" 1}"));
		EXPECT_FALSE(json_read_string("{\"a\": }"));
		EXPECT_FALSE(json_read_string("{\"a\": 1,}"));
		EXPECT_FALSE(json_read_string("{\"a\": 1 ,}"));
		EXPECT_FALSE(json_read_string("{\"a\": 1 , }"));

		EXPECT_FALSE(json_read_string("{\"duplicate\": 1, \"duplicate\": 2}"));

		EXPECT_FALSE(json_read_string("{12: \"key type not string\"}"));
	}

	TEST(JSON, Nested)
	{
		Variant root = json_read_string(R"({"a": [[1], [2, 3], {"aa": 4, "ab": {"abc": 5 }}]})")
		                       .value_or(Variant::map());

		ASSERT_TRUE(root.has("a"));
		ASSERT_TRUE(root.get("a")->is_vector());
		const Variant::Vector &av = root.get("a")->as_vector();
		ASSERT_EQ(3, av.size());

		ASSERT_TRUE(av[0].is_vector());
		const Variant::Vector &av0 = av[0].as_vector();
		ASSERT_EQ(1, av0.size());
		ASSERT_TRUE(av0[0].is_double());
		EXPECT_NEAR(1, av0[0].as_double(), 1e-100);

		ASSERT_TRUE(av[1].is_vector());
		const Variant::Vector &av1 = av[1].as_vector();
		ASSERT_EQ(2, av1.size());
		ASSERT_TRUE(av1[0].is_double());
		EXPECT_NEAR(2, av1[0].as_double(), 1e-100);
		ASSERT_TRUE(av1[1].is_double());
		EXPECT_NEAR(3, av1[1].as_double(), 1e-100);

		ASSERT_TRUE(av[2].has("aa"));
		ASSERT_TRUE(av[2].get("aa")->is_double());
		EXPECT_NEAR(4, av[2].get("aa")->as_double(), 1e-100);
		ASSERT_TRUE(av[2].has("ab"));
		ASSERT_TRUE(av[2].get("ab")->has("abc"));
		ASSERT_TRUE(av[2].get("ab")->get("abc")->is_double());
		EXPECT_NEAR(5, av[2].get("ab")->get("abc")->as_double(), 1e-100);
	}

	TEST(JSON, LeadingAndTrailingSpaceIgnored)
	{
		EXPECT_TRUE(json_read_string("true\n").value_or(Variant(false)).as_bool());
		EXPECT_TRUE(json_read_string("\ntrue").value_or(Variant(false)).as_bool());
		EXPECT_TRUE(json_read_string("\ntrue\n").value_or(Variant(false)).as_bool());
		EXPECT_TRUE(json_read_string(" \n   \r\n \t true \n  \r\n \t ").value_or(Variant(false)).as_bool());
		EXPECT_TRUE(json_read_string(" \n   \r\n \t true \n  \r\n \t ").value_or(Variant(false)).as_bool());
	}

	TEST(JSON, TrailingGarbage)
	{
		EXPECT_FALSE(json_read_string("true true"));
		EXPECT_FALSE(json_read_string("true \ntrue"));
		EXPECT_FALSE(json_read_string("true \ntrue "));
		EXPECT_FALSE(json_read_string("true \n true "));
	}
}
