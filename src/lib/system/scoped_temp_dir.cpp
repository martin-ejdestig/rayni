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

#include <cerrno>
#include <cstdlib>
#include <system_error>
#include <vector>

#include "lib/log.h"

namespace Rayni
{
	Result<ScopedTempDir> ScopedTempDir::create()
	{
		// Cannot be done in a race-free manner with std::filesystem. It is not possible to
		// determine if directory already existed with std::filesystem::create_directory().
		// Checking before with std::filesystem::exists() introduces a race condition.
		//
		// TODO: Will there be something for this in the standard?
		// TODO: Ugh, a lot of copying going on before and after mkdtemp(). ScopedTempDir is
		//       only used in tests at the moment, so ignore for now.
		std::filesystem::path template_path = std::filesystem::temp_directory_path() / "XXXXXX";

		std::vector<char> buffer(template_path.native().cbegin(), template_path.native().cend());
		buffer.push_back('\0');

		if (!mkdtemp(buffer.data()))
			return Error("mkdtemp() failed", std::error_code(errno, std::system_category()));

		ScopedTempDir dir;
		dir.path_ = std::filesystem::path(buffer.data());
		return dir;
	}

	ScopedTempDir::~ScopedTempDir()
	{
		if (!path_.empty()) {
			std::error_code error_code;

			std::filesystem::remove_all(path_, error_code); // noexcept, safe in destructor
			if (error_code)
				log_error("Failed to remove %s: %s", path_.c_str(), error_code.message().c_str());
		}
	}
}
