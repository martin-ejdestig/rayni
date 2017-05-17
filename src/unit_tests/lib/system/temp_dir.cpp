/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2017 Martin Ejdestig <marejde@gmail.com>
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

#include <experimental/filesystem>

#include "lib/system/temp_dir.h"

namespace Rayni
{
	TEST(TempDir, CreateUnique)
	{
		std::experimental::filesystem::path path = temp_dir_create_unique();

		EXPECT_TRUE(std::experimental::filesystem::is_directory(path));
		EXPECT_TRUE(std::experimental::filesystem::remove(path)); // Must be empty.
	}
}
