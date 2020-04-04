// This file is part of Rayni.
//
// Copyright (C) 2013-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/command.h"

#include <gtest/gtest.h>

namespace Rayni
{
	TEST(Command, Stdout)
	{
		auto result = command_run({"echo", "123"});
		ASSERT_TRUE(result);
		EXPECT_EQ("123\n", result->stdout);
		EXPECT_EQ("", result->stderr);
		EXPECT_EQ(0, result->exit_code);
	}

	TEST(Command, Stderr)
	{
		auto result = command_run({"sh", "-c", "echo 123 >&2"});
		ASSERT_TRUE(result);
		EXPECT_EQ("", result->stdout);
		EXPECT_EQ("123\n", result->stderr);
		EXPECT_EQ(0, result->exit_code);
	}

	TEST(Command, ExitCodeOtherThan0)
	{
		auto result = command_run({"sh", "-c", "exit 12"});
		ASSERT_TRUE(result);
		EXPECT_EQ("", result->stdout);
		EXPECT_EQ("", result->stderr);
		EXPECT_EQ(12, result->exit_code);
	}

	TEST(Command, DoesNotExist)
	{
		EXPECT_FALSE(command_run({"does_not_exist"}));
	}
}
