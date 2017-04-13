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

#include "lib/system/temp_dir.h"

#include <cerrno>
#include <cstdlib>
#include <system_error>
#include <vector>

namespace Rayni
{
	// TODO: Possible to implement with final version of C++17 std::filesystem?
	//
	// If so, do it in temp_dir.cpp and remove OS specific temp_dir_*.cpp files.
	//
	// Cannot be done in a race-free manner with std::filesystem in draft. It is not possible
	// to determine if directory already existed with std::filesystem::create_directory().
	// Checking before with std::filesystem::exists() introduces a race condition.
	//
	// Or will there be something for this in the standard? Boost has unique_path() but I
	// suspect the standard group removed it from the TS since they found it inadequate.
	std::experimental::filesystem::path temp_dir_create_unique()
	{
		std::experimental::filesystem::path template_path =
		        std::experimental::filesystem::temp_directory_path() / "XXXXXX";

		std::vector<char> buffer(template_path.native().cbegin(), template_path.native().cend());
		buffer.push_back('\0');

		if (!mkdtemp(buffer.data()))
		{
			std::error_code error_code(errno, std::system_category());
			throw std::experimental::filesystem::filesystem_error("mkdtemp() failed",
			                                                      template_path,
			                                                      error_code);
		}

		return std::experimental::filesystem::path(buffer.data());
	}
}
