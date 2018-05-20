/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/io/file.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "lib/io/io_exception.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	TEST(File, Write)
	{
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "foo";
		const std::vector<std::uint8_t> write_data = {0x12, 0x34};

		file_write(path, write_data);

		std::ifstream file(path, std::ios::binary);
		std::vector<std::uint8_t> read_data((std::istreambuf_iterator<char>(file)),
		                                    std::istreambuf_iterator<char>());

		EXPECT_EQ(write_data, read_data);

		EXPECT_THROW(file_write(temp_dir.path() / "dir_that_does_not_exist" / "bar", write_data), IOException);
	}
}
