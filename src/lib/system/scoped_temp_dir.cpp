/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016 Martin Ejdestig <marejde@gmail.com>
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

#include <experimental/filesystem>
#include <iostream>
#include <system_error>

#include "lib/system/temp_dir.h"

namespace Rayni
{
	ScopedTempDir::ScopedTempDir() : path(temp_dir_create_unique())
	{
	}

	ScopedTempDir::~ScopedTempDir()
	{
		std::error_code error_code;

		std::experimental::filesystem::remove_all(path, error_code); // noexcept, safe in destructor
		if (error_code)
			std::cerr << "Failed to remove " << path << ": " << error_code.message() << '\n';
	}
}
