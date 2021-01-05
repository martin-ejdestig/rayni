// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/linux/pipe.h"

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <utility>

namespace Rayni
{
	TEST(Pipe, FDs)
	{
		Pipe pipe = Pipe::create().value_or({});

		EXPECT_NE(-1, pipe.read_fd());
		EXPECT_NE(-1, pipe.write_fd());
		EXPECT_NE(pipe.read_fd(), pipe.write_fd());
	}

	TEST(Pipe, Close)
	{
		Pipe p1 = Pipe::create().value_or({});
		ASSERT_TRUE(p1.read_fd() != -1 && p1.write_fd() != -1);
		p1.close_read_fd();
		EXPECT_EQ(-1, p1.read_fd());
		EXPECT_NE(-1, p1.write_fd());

		Pipe p2 = Pipe::create().value_or({});
		ASSERT_TRUE(p2.read_fd() != -1 && p2.write_fd() != -1);
		p2.close_write_fd();
		EXPECT_NE(-1, p2.read_fd());
		EXPECT_EQ(-1, p2.write_fd());

		Pipe p3 = Pipe::create().value_or({});
		ASSERT_TRUE(p3.read_fd() != -1 && p3.write_fd() != -1);
		p3.close_fds();
		EXPECT_EQ(-1, p3.read_fd());
		EXPECT_EQ(-1, p3.write_fd());
	}

	TEST(Pipe, ReadWrite)
	{
		Pipe pipe = Pipe::create().value_or({});
		ASSERT_TRUE(pipe.read_fd() != -1 && pipe.write_fd() != -1);

		const std::array<char, 2> write_buffer = {12, 34};
		ASSERT_EQ(2, pipe.write(write_buffer).value_or(0));

		std::array<char, 2> read_buffer;
		ASSERT_EQ(2, pipe.read(read_buffer).value_or(0));
		EXPECT_EQ(write_buffer, read_buffer);

		pipe.close_fds();

		EXPECT_FALSE(pipe.write(write_buffer));
		EXPECT_FALSE(pipe.read(read_buffer));
	}

	TEST(Pipe, ReadAppendToString)
	{
		Pipe pipe = Pipe::create().value_or({});
		ASSERT_TRUE(pipe.read_fd() != -1 && pipe.write_fd() != -1);

		std::string str;

		ASSERT_EQ(3, pipe.write(std::string("abc")).value_or(0));
		EXPECT_EQ(3, pipe.read_append_to_string(str).value_or(0));
		EXPECT_EQ("abc", str);

		ASSERT_EQ(3, pipe.write(std::string("123")).value_or(0));
		ASSERT_EQ(3, pipe.write(std::string("d\0e", 3)).value_or(0));
		ASSERT_EQ(6, pipe.read_append_to_string(str).value_or(0));

		EXPECT_EQ(9, str.length());
		EXPECT_EQ(std::string("abc123d\0e", 9), str);
	}
}
