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

#include "lib/io/binary_reader.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

namespace Rayni
{
	Result<void> BinaryReader::open_file(const std::string &file_name)
	{
		close();

		if (auto result = mmap_file_.map(file_name); !result)
			return result.error();

		buffer_ = static_cast<const std::uint8_t *>(mmap_file_.data());
		buffer_size_ = mmap_file_.size();
		buffer_position_ = 0;

		position_prefix_ = file_name;

		return {};
	}

	void BinaryReader::set_data(std::vector<std::uint8_t> &&data, const std::string &position_prefix)
	{
		close();

		data_ = std::move(data);

		buffer_ = data_.data();
		buffer_size_ = data_.size();
		buffer_position_ = 0;

		position_prefix_ = position_prefix;
	}

	void BinaryReader::close()
	{
		if (mmap_file_.data())
			mmap_file_.unmap();
		else
			data_ = std::vector<std::uint8_t>();

		buffer_ = nullptr;
		buffer_size_ = 0;
		buffer_position_ = 0;

		position_prefix_ = "";
	}

	Result<void> BinaryReader::read_bytes(void *dest,
	                                      std::size_t dest_size,
	                                      std::size_t dest_offset,
	                                      std::size_t num_bytes)
	{
		if (dest_offset >= dest_size)
			return Error(position(),
			             "invalid offset (size: " + std::to_string(dest_size) +
			                     ", offset: " + std::to_string(dest_offset) + ")");

		std::size_t max_num_bytes = dest_size - dest_offset;

		if (num_bytes > max_num_bytes)
			return Error(position(),
			             "byte count too large (byte count: " + std::to_string(num_bytes) +
			                     ", max: " + std::to_string(max_num_bytes) + ")");

		if (buffer_position_ + num_bytes > buffer_size_)
			return Error(position(), "unexpected end of stream");

		std::memcpy(static_cast<std::uint8_t *>(dest) + dest_offset, buffer_ + buffer_position_, num_bytes);

		buffer_position_ += num_bytes;

		return {};
	}

	std::string BinaryReader::position() const
	{
		if (!buffer_)
			return "";

		std::string str;

		if (!position_prefix_.empty())
			str += position_prefix_;

		if (buffer_position_ <= buffer_size_) {
			if (!str.empty())
				str += ":";
			str += "<offset " + std::to_string(buffer_position_) + ">";
		}

		return str;
	}
}
