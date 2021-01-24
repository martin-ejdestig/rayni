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

#include "lib/io/file.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	TEST(File, ReadAndWrite)
	{
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());
		const std::string path = temp_dir.path() / "foo";

		const std::vector<std::uint8_t> write_data = {0x12, 0x34};
		ASSERT_TRUE(file_write(path, write_data));

		Result<std::vector<std::uint8_t>> read_data = file_read(path);
		ASSERT_TRUE(read_data);

		EXPECT_EQ(write_data, *read_data);
	}

	TEST(File, ReadAndWriteZeroBytes)
	{
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());
		const std::string path = temp_dir.path() / "foo";

		ASSERT_TRUE(file_write(path, {}));

		Result<std::vector<std::uint8_t>> read_data = file_read(path);
		ASSERT_TRUE(read_data);

		EXPECT_EQ(0, read_data->size());
	}

	TEST(File, ReadAndWriteInNonexistingDirFails)
	{
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

		EXPECT_FALSE(file_read(temp_dir.path() / "dir_that_does_not_exist" / "bar"));
		EXPECT_FALSE(file_write(temp_dir.path() / "dir_that_does_not_exist" / "baz", {0, 1, 2, 3}));
	}

}
