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

#include "lib/system/scoped_temp_dir.h"

#include <gtest/gtest.h>

#include <filesystem>

#include "lib/io/file.h"

namespace Rayni
{
	TEST(ScopedTempDir, RemovedWhenDestroyed)
	{
		std::filesystem::path path;

		{
			ScopedTempDir dir = ScopedTempDir::create().value_or({});
			ASSERT_FALSE(dir.path().empty());

			path = dir.path();

			ASSERT_TRUE(file_write(path / "foo", {0, 1, 2, 3}));

			std::filesystem::create_directory(path / "bar");
			ASSERT_TRUE(file_write(path / "bar" / "baz", {4, 5, 6, 7}));
		}

		EXPECT_FALSE(std::filesystem::exists(path));
	}
}
