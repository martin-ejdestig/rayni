// This file is part of Rayni.
//
// Copyright (C) 2016-2019 Martin Ejdestig <marejde@gmail.com>
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

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

namespace
{
	// Cannot be done in a race-free manner with std::filesystem. It is not possible to
	// determine if directory already existed with std::filesystem::create_directory().
	// Checking before with std::filesystem::exists() introduces a race condition.
	//
	// TODO: Will there be something for this in the standard?
	std::filesystem::path temp_dir_create_unique()
	{
		std::filesystem::path template_path = std::filesystem::temp_directory_path() / "XXXXXX";

		std::vector<char> buffer(template_path.native().cbegin(), template_path.native().cend());
		buffer.push_back('\0');

		if (!mkdtemp(buffer.data()))
		{
			std::error_code error_code(errno, std::system_category());
			throw std::filesystem::filesystem_error("mkdtemp() failed", template_path, error_code);
		}

		return std::filesystem::path(buffer.data());
	}
}

namespace Rayni
{
	ScopedTempDir::ScopedTempDir() : path_(temp_dir_create_unique())
	{
	}

	ScopedTempDir::~ScopedTempDir()
	{
		std::error_code error_code;

		std::filesystem::remove_all(path(), error_code); // noexcept, safe in destructor
		if (error_code)
			std::cerr << "Failed to remove " << path() << ": " << error_code.message() << '\n';
	}
}
