/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/scoped_temp_dir.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace Rayni
{
	TEST(ScopedTempDir, RemovedWhenDestroyed)
	{
		auto output_to_file = [](auto path, auto content) {
			std::ofstream stream(path);
			stream << content;
			if (!stream.good())
				FAIL() << "Failed to write to file " << path << ".";
		};

		std::filesystem::path path;

		{
			ScopedTempDir dir;
			path = dir.path();

			output_to_file(path / "foo", "bla bla");

			std::filesystem::create_directory(path / "bar");
			output_to_file(path / "bar" / "baz", "yada yada");
		}

		EXPECT_FALSE(std::filesystem::exists(path));
	}
}
