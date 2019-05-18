// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_SYSTEM_MEMORY_MAPPED_FILE_H
#define RAYNI_LIB_SYSTEM_MEMORY_MAPPED_FILE_H

#include <cstddef>
#include <string>
#include <utility>

#include "lib/system/unique_fd.h"

namespace Rayni
{
	class MemoryMappedFile
	{
	public:
		MemoryMappedFile() = default;

		explicit MemoryMappedFile(const std::string &file_name);

		MemoryMappedFile(const MemoryMappedFile &other) = delete;

		MemoryMappedFile(MemoryMappedFile &&other) noexcept :
		        data_(std::exchange(other.data_, nullptr)),
		        size_(std::exchange(other.size_, 0))
		{
		}

		~MemoryMappedFile();

		MemoryMappedFile &operator=(const MemoryMappedFile &other) = delete;

		MemoryMappedFile &operator=(MemoryMappedFile &&other) noexcept
		{
			unmap();

			data_ = std::exchange(other.data_, nullptr);
			size_ = std::exchange(other.size_, 0);

			return *this;
		}

		void map(const std::string &file_name);
		void unmap() noexcept;

		const void *data() const
		{
			return data_;
		}

		std::size_t size() const
		{
			return size_;
		}

	private:
		void *data_ = nullptr;
		std::size_t size_ = 0;
	};
}

#endif // RAYNI_LIB_SYSTEM_MEMORY_MAPPED_FILE_H
