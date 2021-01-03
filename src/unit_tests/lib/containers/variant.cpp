// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/containers/variant.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lib/function/result.h"

namespace Rayni
{
	TEST(Variant, Is)
	{
		EXPECT_TRUE(Variant().is_none());
		EXPECT_TRUE(Variant::vector().is_vector());
		EXPECT_TRUE(Variant::map().is_map());
		EXPECT_TRUE(Variant(false).is_bool());
		EXPECT_TRUE(Variant(0).is_int());
		EXPECT_TRUE(Variant(0U).is_unsigned_int());
		EXPECT_TRUE(Variant(0.0F).is_float());
		EXPECT_TRUE(Variant(0.0).is_double());
		EXPECT_TRUE(Variant("").is_string());

		EXPECT_FALSE(Variant(0).is_none());
		EXPECT_FALSE(Variant().is_map());
		EXPECT_FALSE(Variant().is_vector());
		EXPECT_FALSE(Variant().is_bool());
		EXPECT_FALSE(Variant().is_int());
		EXPECT_FALSE(Variant().is_unsigned_int());
		EXPECT_FALSE(Variant().is_float());
		EXPECT_FALSE(Variant().is_double());
		EXPECT_FALSE(Variant().is_string());
	}

	TEST(Variant, Move)
	{
		Variant v1(true);
		EXPECT_TRUE(v1.is_bool());

		Variant v2(std::move(v1));

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_TRUE(v1.is_none());
		EXPECT_TRUE(v2.is_bool());

		Variant v3 = std::move(v2);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_TRUE(v2.is_none());
		EXPECT_TRUE(v3.is_bool());
	}

	TEST(Variant, As)
	{
		EXPECT_EQ(0, Variant::map().as_map().size());
		EXPECT_EQ(0, Variant::vector().as_vector().size());
		EXPECT_EQ(true, Variant(true).as_bool());
		EXPECT_EQ(1, Variant(1).as_int());
		EXPECT_EQ(1U, Variant(1U).as_unsigned_int());
		EXPECT_FLOAT_EQ(1.0F, Variant(1.0F).as_float());
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0).as_double());
		EXPECT_EQ("abc", Variant("abc").as_string());
	}

	TEST(Variant, To)
	{
		EXPECT_EQ(true, Variant(true).to_bool().value_or(false));
		EXPECT_FALSE(Variant().to_bool());

		EXPECT_EQ(1, Variant(1).to_int().value_or(0));
		EXPECT_EQ(1, Variant(1U).to_int().value_or(0));
		EXPECT_EQ(1, Variant(1.0F).to_int().value_or(0));
		EXPECT_EQ(1, Variant(1.0).to_int().value_or(0));
		EXPECT_FALSE(Variant().to_int());

		EXPECT_EQ(1U, Variant(1).to_unsigned_int().value_or(0));
		EXPECT_EQ(1U, Variant(1U).to_unsigned_int().value_or(0));
		EXPECT_EQ(1U, Variant(1.0F).to_unsigned_int().value_or(0));
		EXPECT_EQ(1U, Variant(1.0).to_unsigned_int().value_or(0));
		EXPECT_FALSE(Variant().to_unsigned_int());

		EXPECT_FLOAT_EQ(1.0F, Variant(1).to_float().value_or(0));
		EXPECT_FLOAT_EQ(1.0F, Variant(1U).to_float().value_or(0));
		EXPECT_FLOAT_EQ(1.0F, Variant(1.0F).to_float().value_or(0));
		EXPECT_FLOAT_EQ(1.0F, Variant(1.0).to_float().value_or(0));
		EXPECT_FALSE(Variant().to_float());

		EXPECT_DOUBLE_EQ(1.0, Variant(1).to_double().value_or(0));
		EXPECT_DOUBLE_EQ(1.0, Variant(1U).to_double().value_or(0));
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0F).to_double().value_or(0));
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0).to_double().value_or(0));
		EXPECT_FALSE(Variant().to_double());

		EXPECT_EQ("{ key1: 123, key2: abc }",
		          Variant::map("key1", 123, "key2", "abc").to_string().value_or(""));

		EXPECT_EQ("[ 123, abc ]", Variant::vector(123, "abc").to_string().value_or(""));

		EXPECT_EQ("1", Variant(1).to_string().value_or(""));
		EXPECT_EQ("1", Variant(1U).to_string().value_or(""));
		EXPECT_EQ(std::to_string(1.0F), Variant(1.0F).to_string().value_or(""));
		EXPECT_EQ(std::to_string(1.0), Variant(1.0).to_string().value_or(""));
		EXPECT_EQ("abc", Variant("abc").to_string().value_or(""));
		EXPECT_FALSE(Variant().to_string());

		EXPECT_EQ(false, Variant(false).to<bool>().value_or(true));
		EXPECT_EQ(123, Variant(123).to<int>().value_or(0));
		EXPECT_EQ(123U, Variant(123).to<unsigned int>().value_or(0));
		EXPECT_FLOAT_EQ(123.0F, Variant(123).to<float>().value_or(0));
		EXPECT_DOUBLE_EQ(123.0, Variant(123).to<double>().value_or(0));
		EXPECT_EQ("123", Variant(123).to<std::string>().value_or(""));
	}

	TEST(Variant, ToFromVariant)
	{
		struct Foo
		{
			explicit Foo(int foo) : bar(foo)
			{
			}

			static Result<Foo> from_variant(const Variant &v)
			{
				auto i = v.to_int();
				if (!i)
					return i.error();
				return Foo(*i);
			}

			int bar = 0;
		};

		EXPECT_EQ(123, Variant(123).to<Foo>().value_or(Foo(1)).bar);
	}

	TEST(Variant, Has)
	{
		Variant variant = Variant::map("key1", 123, "key2", "abc");

		EXPECT_TRUE(variant.has("key1"));
		EXPECT_TRUE(variant.has("key2"));
		EXPECT_FALSE(variant.has("key3"));

		EXPECT_FALSE(Variant().has("key"));
		EXPECT_FALSE(Variant::vector().has("key"));
		EXPECT_FALSE(Variant(false).has("key"));
		EXPECT_FALSE(Variant(0).has("key"));
		EXPECT_FALSE(Variant(0U).has("key"));
		EXPECT_FALSE(Variant(0.0F).has("key"));
		EXPECT_FALSE(Variant(0.0).has("key"));
		EXPECT_FALSE(Variant("").has("key"));
	}

	TEST(Variant, GetFromMap)
	{
		Variant variant = Variant::map("key1", 123, "key2", "abc");

		ASSERT_NE(nullptr, variant.get("key1"));
		ASSERT_TRUE(variant.get("key1")->is_int());
		EXPECT_EQ(123, variant.get("key1")->as_int());
		EXPECT_EQ(123, variant.get<int>("key1").value_or(0));
		EXPECT_EQ(123, variant.get("key1", 456).value_or(0));
		EXPECT_EQ(456, variant.get("key3", 456).value_or(0));

		ASSERT_NE(nullptr, variant.get("key2"));
		ASSERT_TRUE(variant.get("key2")->is_string());
		EXPECT_EQ("abc", variant.get("key2")->as_string());
		EXPECT_EQ("abc", variant.get<std::string>("key2").value_or(""));
		EXPECT_EQ("abc", variant.get("key2", std::string("def")).value_or(""));
		EXPECT_EQ("def", variant.get("key3", std::string("def")).value_or(""));

		EXPECT_EQ(nullptr, variant.get("key3"));
	}

	TEST(Variant, GetFromVector)
	{
		Variant variant = Variant::vector(123, "abc");

		ASSERT_NE(nullptr, variant.get(0));
		ASSERT_TRUE(variant.get(0)->is_int());
		EXPECT_EQ(123, variant.get(0)->as_int());
		EXPECT_EQ(123, variant.get<int>(0).value_or(0));

		ASSERT_NE(nullptr, variant.get(1));
		ASSERT_TRUE(variant.get(1)->is_string());
		EXPECT_EQ("abc", variant.get(1)->as_string());
		EXPECT_EQ("abc", variant.get<std::string>(1).value_or(""));

		EXPECT_EQ(nullptr, variant.get(2));
	}

	TEST(Variant, Path)
	{
		Variant variant;

		EXPECT_EQ("", variant.path());

		variant = Variant::map("key1", Variant::vector(123, "abc"), "key2", Variant::vector(456, "def"));
		EXPECT_EQ("", variant.path());
		EXPECT_EQ("['key1']", variant.get("key1")->path());
		EXPECT_EQ("['key2']", variant.get("key2")->path());
		EXPECT_EQ("['key1'][0]", variant.get("key1")->get(0)->path());
		EXPECT_EQ("['key1'][1]", variant.get("key1")->get(1)->path());
		EXPECT_EQ("['key2'][0]", variant.get("key2")->get(0)->path());
		EXPECT_EQ("['key2'][1]", variant.get("key2")->get(1)->path());

		variant = Variant::vector(Variant::map("key1", 123, "key2", "abc"),
		                          Variant::map("key1", 456, "key2", "def"));
		EXPECT_EQ("", variant.path());
		EXPECT_EQ("[0]", variant.get(0)->path());
		EXPECT_EQ("[1]", variant.get(1)->path());
		EXPECT_EQ("[0]['key1']", variant.get(0)->get("key1")->path());
		EXPECT_EQ("[0]['key2']", variant.get(0)->get("key2")->path());
		EXPECT_EQ("[1]['key1']", variant.get(1)->get("key1")->path());
		EXPECT_EQ("[1]['key2']", variant.get(1)->get("key2")->path());
	}
}
