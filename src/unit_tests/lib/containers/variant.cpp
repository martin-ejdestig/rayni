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

#include <string>
#include <utility>
#include <vector>

#include "lib/containers/variant.h"

namespace Rayni
{
	TEST(VariantTest, Is)
	{
		EXPECT_TRUE(Variant().is_none());

		EXPECT_TRUE(Variant::vector().is_vector());
		EXPECT_TRUE(Variant(Variant::Map()).is_map());
		EXPECT_TRUE(Variant(false).is_bool());
		EXPECT_TRUE(Variant(0).is_int());
		EXPECT_TRUE(Variant(0u).is_unsigned_int());
		EXPECT_TRUE(Variant(0.0f).is_float());
		EXPECT_TRUE(Variant(0.0).is_double());
		EXPECT_TRUE(Variant("").is_string());

		EXPECT_TRUE(Variant::vector().is<Variant::Vector>());
		EXPECT_TRUE(Variant(Variant::Map()).is<Variant::Map>());
		EXPECT_TRUE(Variant(false).is<bool>());
		EXPECT_TRUE(Variant(0).is<int>());
		EXPECT_TRUE(Variant(0u).is<unsigned int>());
		EXPECT_TRUE(Variant(0.0f).is<float>());
		EXPECT_TRUE(Variant(0.0).is<double>());
		EXPECT_TRUE(Variant("").is<std::string>());
	}

	TEST(VariantTest, Move)
	{
		Variant v1(true);
		EXPECT_TRUE(v1.is_bool());

		Variant v2(std::move(v1));
		EXPECT_TRUE(v1.is_none());
		EXPECT_TRUE(v2.is_bool());

		Variant v3 = std::move(v2);
		EXPECT_TRUE(v2.is_none());
		EXPECT_TRUE(v3.is_bool());
	}

	TEST(VariantTest, GetValue)
	{
		EXPECT_EQ(0, Variant(Variant::Map()).get_map().size());
		EXPECT_EQ(0, Variant::vector().get_vector().size());
		EXPECT_EQ(true, Variant(true).get_bool());
		EXPECT_EQ(1, Variant(1).get_int());
		EXPECT_EQ(1u, Variant(1u).get_unsigned_int());
		EXPECT_FLOAT_EQ(1.0f, Variant(1.0f).get_float());
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0).get_double());
		EXPECT_EQ("abc", Variant("abc").get_string());

		EXPECT_EQ(0, Variant(Variant::Map()).get_value<Variant::Map>().size());
		EXPECT_EQ(0, Variant::vector().get_value<Variant::Vector>().size());
		EXPECT_EQ(true, Variant(true).get_value<bool>());
		EXPECT_EQ(1, Variant(1).get_value<int>());
		EXPECT_EQ(1u, Variant(1u).get_value<unsigned int>());
		EXPECT_FLOAT_EQ(1.0f, Variant(1.0f).get_value<float>());
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0).get_value<double>());
		EXPECT_EQ("abc", Variant("abc").get_value<std::string>());

		EXPECT_THROW(Variant().get_map(), Variant::Exception);
		EXPECT_THROW(Variant().get_vector(), Variant::Exception);
		EXPECT_THROW(Variant().get_bool(), Variant::Exception);
		EXPECT_THROW(Variant().get_int(), Variant::Exception);
		EXPECT_THROW(Variant().get_unsigned_int(), Variant::Exception);
		EXPECT_THROW(Variant().get_float(), Variant::Exception);
		EXPECT_THROW(Variant().get_double(), Variant::Exception);
		EXPECT_THROW(Variant().get_string(), Variant::Exception);
	}

	TEST(VariantTest, To)
	{
		EXPECT_EQ(1, Variant(1).to_int());
		EXPECT_EQ(1, Variant(1u).to_int());
		EXPECT_EQ(1, Variant(1.0f).to_int());
		EXPECT_EQ(1, Variant(1.0).to_int());
		EXPECT_THROW(Variant().to_int(), Variant::Exception);

		EXPECT_EQ(1u, Variant(1).to_unsigned_int());
		EXPECT_EQ(1u, Variant(1u).to_unsigned_int());
		EXPECT_EQ(1u, Variant(1.0f).to_unsigned_int());
		EXPECT_EQ(1u, Variant(1.0).to_unsigned_int());
		EXPECT_THROW(Variant().to_unsigned_int(), Variant::Exception);

		EXPECT_FLOAT_EQ(1.0f, Variant(1).to_float());
		EXPECT_FLOAT_EQ(1.0f, Variant(1u).to_float());
		EXPECT_FLOAT_EQ(1.0f, Variant(1.0f).to_float());
		EXPECT_FLOAT_EQ(1.0f, Variant(1.0).to_float());
		EXPECT_THROW(Variant().to_float(), Variant::Exception);

		EXPECT_DOUBLE_EQ(1.0, Variant(1).to_double());
		EXPECT_DOUBLE_EQ(1.0, Variant(1u).to_double());
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0f).to_double());
		EXPECT_DOUBLE_EQ(1.0, Variant(1.0).to_double());
		EXPECT_THROW(Variant().to_double(), Variant::Exception);

		EXPECT_EQ("{ key1: 123, key2: abc }", Variant::map("key1", 123, "key2", "abc").to_string());

		EXPECT_EQ("[ 123, abc ]", Variant::vector(123, "abc").to_string());

		EXPECT_EQ("1", Variant(1).to_string());
		EXPECT_EQ("1", Variant(1u).to_string());
		EXPECT_EQ(std::to_string(1.0f), Variant(1.0f).to_string());
		EXPECT_EQ(std::to_string(1.0), Variant(1.0).to_string());
		EXPECT_EQ("abc", Variant("abc").to_string());
		EXPECT_THROW(Variant().to_string(), Variant::Exception);

		EXPECT_EQ(123, Variant(123).to<int>());
		EXPECT_EQ(123u, Variant(123).to<unsigned int>());
		EXPECT_FLOAT_EQ(123.0f, Variant(123).to<float>());
		EXPECT_DOUBLE_EQ(123.0, Variant(123).to<double>());
		EXPECT_EQ("123", Variant(123).to<std::string>());
	}

	TEST(VariantTest, ToConstructor)
	{
		struct Foo
		{
			explicit Foo(const Variant &v) : bar(v.to_int())
			{
			}

			int bar = 0;
		};

		EXPECT_EQ(123, Variant(123).to<Foo>().bar);
	}

	TEST(VariantTest, ToFromVariant)
	{
		struct Foo
		{
			explicit Foo(int foo) : bar(foo)
			{
			}

			static Foo from_variant(const Variant &v)
			{
				return Foo(v.to_int());
			}

			int bar = 0;
		};

		EXPECT_EQ(123, Variant(123).to<Foo>().bar);
	}

	TEST(VariantTest, ToGetFromVariant)
	{
		struct Foo
		{
			explicit Foo(int foo) : bar(foo)
			{
			}

			static Foo *get_from_variant(const Variant &v)
			{
				static std::vector<Foo> foos;
				int bar = v.to_int();

				for (auto &foo : foos)
					if (foo.bar == bar)
						return &foo;

				foos.emplace_back(bar);

				return &foos.back();
			}

			int bar = 0;
		};

		EXPECT_EQ(123, Variant(123).to<Foo *>()->bar);
	}

	TEST(VariantTest, GetFromMap)
	{
		auto variant = Variant::map("key1", 123, "key2", "abc");

		EXPECT_EQ(123, variant.get("key1").get_int());
		EXPECT_EQ(123, variant.get<int>("key1"));
		EXPECT_EQ(123, variant.get("key1", 456));
		EXPECT_EQ(456, variant.get("key3", 456));

		EXPECT_EQ("abc", variant.get("key2").get_string());
		EXPECT_EQ("abc", variant.get<std::string>("key2"));
		EXPECT_EQ("abc", variant.get("key2", std::string("def")));
		EXPECT_EQ("def", variant.get("key3", std::string("def")));

		EXPECT_THROW(variant.get("key3"), Variant::Exception);
	}

	TEST(VariantTest, GetFromVector)
	{
		Variant variant = Variant::vector(123, "abc");

		EXPECT_EQ(123, variant.get(0).get_int());
		EXPECT_EQ(123, variant.get<int>(0));

		EXPECT_EQ("abc", variant.get(1).get_string());
		EXPECT_EQ("abc", variant.get<std::string>(1));

		EXPECT_THROW(variant.get(2), Variant::Exception);
	}

	TEST(VariantTest, GetPath)
	{
		Variant::Map map;
		Variant variant;

		EXPECT_EQ("", variant.get_path());

		map.emplace("key1", Variant::vector(123, "abc"));
		map.emplace("key2", Variant::vector(456, "def"));
		variant = Variant(std::move(map));

		EXPECT_EQ("['key1']", variant.get("key1").get_path());
		EXPECT_EQ("['key2']", variant.get("key2").get_path());
		EXPECT_EQ("['key1'][0]", variant.get("key1").get(0).get_path());
		EXPECT_EQ("['key1'][1]", variant.get("key1").get(1).get_path());
		EXPECT_EQ("['key2'][0]", variant.get("key2").get(0).get_path());
		EXPECT_EQ("['key2'][1]", variant.get("key2").get(1).get_path());

		variant = Variant::vector(Variant::map("key1", 123, "key2", "abc"),
		                          Variant::map("key1", 456, "key2", "def"));
		EXPECT_EQ("", variant.get_path());
		EXPECT_EQ("[0]", variant.get(0).get_path());
		EXPECT_EQ("[1]", variant.get(1).get_path());
		EXPECT_EQ("[0]['key1']", variant.get(0).get("key1").get_path());
		EXPECT_EQ("[0]['key2']", variant.get(0).get("key2").get_path());
		EXPECT_EQ("[1]['key1']", variant.get(1).get("key1").get_path());
		EXPECT_EQ("[1]['key2']", variant.get(1).get("key2").get_path());
	}
}
