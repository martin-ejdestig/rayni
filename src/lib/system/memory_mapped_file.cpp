// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/system/memory_mapped_file.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <system_error>

namespace Rayni
{
	MemoryMappedFile::~MemoryMappedFile()
	{
		unmap();
	}

	Result<void> MemoryMappedFile::map(const std::string &file_name)
	{
		unmap();

		UniqueFD fd(open(file_name.c_str(), O_RDONLY));
		if (fd.get() == -1)
			return Error(file_name + ": failed to open file",
			             std::error_code(errno, std::system_category()));

		struct stat stat;
		if (fstat(fd.get(), &stat) != 0)
			return Error(file_name + ": failed to stat file",
			             std::error_code(errno, std::system_category()));

		auto size = static_cast<std::size_t>(stat.st_size);
		void *ptr = nullptr;

		if (size != 0) {
			ptr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd.get(), 0);
			if (ptr == MAP_FAILED)
				return Error(file_name + ": failed to map file",
				             std::error_code(errno, std::system_category()));
		}

		data_ = ptr;
		size_ = size;

		return {};
	}

	void MemoryMappedFile::unmap() noexcept
	{
		if (!data_)
			return;

		munmap(data_, size_);
		data_ = nullptr;
		size_ = 0;
	}
}
