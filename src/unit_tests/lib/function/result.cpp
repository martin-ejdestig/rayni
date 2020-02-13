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

#include "lib/function/result.h"

#include <gtest/gtest.h>

#include <string>
#include <system_error>

namespace Rayni
{
	namespace
	{
		struct Image
		{
			unsigned int width = 0;
			unsigned int height = 0;
		};

		Result<void> void_result(int x)
		{
			if (x <= 0)
				return Error("x <= 0");

			// Do something with positive x...

			return {};
		}

		Result<int> add_positive(int x, int y)
		{
			if (x < 0)
				return Error("x < 0");

			if (y < 0)
				return Error("y < 0");

			return x + y;
		}

		// NOLINTNEXTLINE(readability-const-return-type) For testing "const T &&value() const &&".
		const Result<int> add_positive_const_rvalue_result(int x, int y)
		{
			return add_positive(x, y);
		}

		Result<int> add_positive_error_code(int x, int y)
		{
			if (x < 0)
				return Error("x < 0", std::make_error_code(std::errc::invalid_argument));

			if (y < 0)
				return Error("y < 0", std::make_error_code(std::errc::invalid_argument));

			return x + y;
		}

		Result<Image> create_image(unsigned int width, unsigned int height)
		{
			if (width == 0 || height == 0)
				return Error("width and height must be > 0");

			Image image;
			image.width = width;
			image.height = height;

			return image;
		}

		// NOLINTNEXTLINE(readability-const-return-type) For testing "const T &&value() const &&".
		const Result<Image> create_image_const_rvalue_result(unsigned int width, unsigned height)
		{
			return create_image(width, height);
		}
	}

	TEST(Result, Void)
	{
		EXPECT_FALSE(void_result(1).is_error());
		EXPECT_TRUE(void_result(-1).is_error());
	}

	TEST(Result, Value)
	{
		ASSERT_FALSE(add_positive(1, 2).is_error());
		EXPECT_TRUE(add_positive(-1, -2).is_error());

		Result<int> r = add_positive(1, 2);
		const Result<int> r_const = r;

		EXPECT_EQ(3, r.value()); // &
		EXPECT_EQ(3, r_const.value()); // const &
		EXPECT_EQ(3, add_positive(1, 2).value()); // &&
		EXPECT_EQ(3, add_positive_const_rvalue_result(1, 2).value()); // const &&
	}

	TEST(Result, ValueStruct)
	{
		ASSERT_FALSE(create_image(2, 2).is_error());
		EXPECT_TRUE(create_image(0, 0).is_error());

		Result<Image> r = create_image(2, 2);
		const Result<Image> r_const = r;

		EXPECT_EQ(2, r.value().width); // &
		EXPECT_EQ(2, r_const.value().width); // const &
		EXPECT_EQ(2, create_image(2, 2).value().width); // &&
		EXPECT_EQ(2, create_image_const_rvalue_result(2, 2).value().width); // const &&
	}

	TEST(Result, ValueOr)
	{
		Result<int> r1 = add_positive(1, 2);
		Result<int> r2 = add_positive(-1, -2);

		EXPECT_EQ(3, r1.value_or(0)); // const &
		EXPECT_EQ(0, r2.value_or(0)); // const &
		EXPECT_EQ(3, add_positive(1, 2).value_or(0)); // &&
		EXPECT_EQ(0, add_positive(-1, -2).value_or(0)); // &&
	}

	TEST(Result, BoolOperator)
	{
		EXPECT_TRUE(void_result(1));
		EXPECT_FALSE(void_result(-1));

		EXPECT_TRUE(add_positive(1, 2));
		EXPECT_FALSE(add_positive(-1, -2));

		EXPECT_TRUE(create_image(2, 2));
		EXPECT_FALSE(create_image(0, 0));
	}

	TEST(Result, IndirectionOperator)
	{
		Result<int> r = add_positive(1, 2);
		EXPECT_EQ(3, *r);

		const Result<int> r_const = r;
		EXPECT_EQ(3, *r_const);
	}

	TEST(Result, StructureDereferenceOperator)
	{
		Result<Image> r = create_image(2, 2);
		EXPECT_EQ(2, r->width);

		const Result<Image> r_const = r;
		EXPECT_EQ(2, r_const->width);
	}

	TEST(Result, ErrorMessage)
	{
		EXPECT_EQ("x <= 0", void_result(-1).error().message());
		EXPECT_EQ("x < 0", add_positive(-1, 1).error().message());
	}

	TEST(Result, ErrorMessageWithErrorCode)
	{
		const std::string prefix = "x < 0: ";
		const std::string message = add_positive_error_code(-1, 1).error().message();

		EXPECT_TRUE(message.compare(0, prefix.size(), prefix) == 0)
		        << "message=\"" << message << "\", prefix=\"" << prefix << "\"";
		EXPECT_GT(message.size(), prefix.size());
	}
}
