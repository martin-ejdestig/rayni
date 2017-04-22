/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <array>
#include <string>
#include <system_error>
#include <utility>

#include "lib/system/linux/pipe.h"

namespace Rayni
{
	TEST(PipeTest, FDs)
	{
		Pipe pipe;

		EXPECT_NE(-1, pipe.read_fd());
		EXPECT_NE(-1, pipe.write_fd());
		EXPECT_NE(pipe.read_fd(), pipe.write_fd());
	}

	TEST(PipeTest, MoveConstructor)
	{
		Pipe p1;

		int read_fd = p1.read_fd();
		int write_fd = p1.write_fd();

		Pipe p2(std::move(p1));

		EXPECT_EQ(-1, p1.read_fd()); // NOLINT: misc-use-after-move (want to test state after move...)
		EXPECT_EQ(-1, p1.write_fd());

		EXPECT_EQ(read_fd, p2.read_fd());
		EXPECT_EQ(write_fd, p2.write_fd());
	}

	TEST(PipeTest, MoveAssignment)
	{
		Pipe p1;

		int read_fd = p1.read_fd();
		int write_fd = p1.write_fd();

		Pipe p2;
		p2 = std::move(p1);

		EXPECT_EQ(-1, p1.read_fd()); // NOLINT: misc-use-after-move (want to test state after move...)
		EXPECT_EQ(-1, p1.write_fd());

		EXPECT_EQ(read_fd, p2.read_fd());
		EXPECT_EQ(write_fd, p2.write_fd());
	}

	TEST(PipeTest, Close)
	{
		Pipe p1;
		p1.close_read_fd();
		EXPECT_EQ(-1, p1.read_fd());
		EXPECT_NE(-1, p1.write_fd());

		Pipe p2;
		p2.close_write_fd();
		EXPECT_NE(-1, p2.read_fd());
		EXPECT_EQ(-1, p2.write_fd());

		Pipe p3;
		p3.close_fds();
		EXPECT_EQ(-1, p3.read_fd());
		EXPECT_EQ(-1, p3.write_fd());
	}

	TEST(PipeTest, ReadWrite)
	{
		Pipe pipe;

		const std::array<char, 2> write_buffer = {12, 34};
		ASSERT_EQ(2, pipe.write(write_buffer));

		std::array<char, 2> read_buffer;
		ASSERT_EQ(2, pipe.read(read_buffer));

		EXPECT_EQ(write_buffer.at(0), read_buffer.at(0));
		EXPECT_EQ(write_buffer.at(1), read_buffer.at(1));

		pipe.close_fds();

		EXPECT_THROW(pipe.write(write_buffer), std::system_error);
		EXPECT_THROW(pipe.read(read_buffer), std::system_error);
	}

	TEST(PipeTest, ReadAppendToString)
	{
		Pipe pipe;
		const std::string write_str1("abc");
		const std::string write_str2("123");
		const std::string write_str3("d\0e", 3);
		std::string str;

		pipe.write(write_str1);

		pipe.read_append_to_string(str);
		EXPECT_EQ("abc", str);

		pipe.write(write_str2);
		pipe.write(write_str3);

		pipe.read_append_to_string(str);
		EXPECT_EQ(9, str.length());
		EXPECT_EQ(std::string("abc123d\0e", 9), str);
	}
}
