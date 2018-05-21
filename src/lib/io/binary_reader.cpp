/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/io/binary_reader.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <ios>
#include <istream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace Rayni
{
	void BinaryReader::open_file(const std::string &file_name)
	{
		auto file = std::make_unique<std::ifstream>(file_name);

		if (!file->is_open())
			throw Exception(file_name, "failed to open file");

		reset(std::move(file), file_name);
	}

	void BinaryReader::set_data(std::vector<std::uint8_t> &&data, const std::string &position_prefix)
	{
		// TODO: Copying data (twice), misuse of std::istringstream and ugly sign conversion.
		//
		// Complexity of implementing an std::istream/std::streambuf that reads from a
		// memory area is not worth it. (Two classes and ugly const casting for
		// std::streambuf::setg. tellg() and seekg() requires overriding at least
		// std::streambuf::seekoff() and handling all permutations of arguments etc.)
		//
		// If performance is really important it is probably better to remove use of
		// std::istream all together and just read memory directly. Reading from file would
		// probably benefit performance wise as well by use of mmap(). Have testing use of
		// MemoryMappedFile when reading large object and image files on the TODO list.
		std::vector<std::uint8_t> data_to_free = std::move(data);
		const void *ptr = data_to_free.data();
		std::string string(static_cast<const char *>(ptr), data_to_free.size());

		reset(std::make_unique<std::istringstream>(std::move(string)), position_prefix);
	}

	void BinaryReader::reset(std::unique_ptr<std::istream> &&istream, const std::string &position_prefix)
	{
		istream_ = std::move(istream);
		position_prefix_ = position_prefix;
	}

	void BinaryReader::close()
	{
		istream_.reset();
		position_prefix_ = "";
	}

	void BinaryReader::read_bytes(void *dest, std::size_t dest_size, std::size_t dest_offset, std::size_t num_bytes)
	{
		if (dest_offset >= dest_size)
			throw Exception(position(),
			                "invalid offset (size: " + std::to_string(dest_size) +
			                        ", offset: " + std::to_string(dest_offset) + ")");

		std::size_t max_num_bytes =
		        std::min(dest_size - dest_offset, std::size_t(std::numeric_limits<std::streamsize>::max()));

		if (num_bytes > max_num_bytes)
			throw Exception(position(),
			                "byte count too large (byte count: " + std::to_string(num_bytes) +
			                        ", max: " + std::to_string(max_num_bytes) + ")");

		static_assert(sizeof(std::istream::char_type) == 1);

		istream_->read(static_cast<std::istream::char_type *>(dest) + dest_offset,
		               static_cast<std::streamsize>(num_bytes));

		if (!istream_->good())
		{
			if (istream_->eof())
				throw Exception(position(), "unexpected end of stream");

			throw Exception(position(), "read error");
		}
	}

	std::uint8_t BinaryReader::read_uint8()
	{
		std::array<std::uint8_t, 1> bytes;
		read_bytes(bytes);
		return bytes[0];
	}

	std::uint16_t BinaryReader::read_big_endian_uint16()
	{
		std::array<std::uint8_t, 2> bytes;
		read_bytes(bytes);
		return static_cast<std::uint16_t>(bytes[0]) << 8 | static_cast<std::uint16_t>(bytes[1]);
	}

	std::uint16_t BinaryReader::read_little_endian_uint16()
	{
		std::array<std::uint8_t, 2> bytes;
		read_bytes(bytes);
		return static_cast<std::uint16_t>(bytes[1]) << 8 | static_cast<std::uint16_t>(bytes[0]);
	}

	std::uint32_t BinaryReader::read_big_endian_uint32()
	{
		std::array<std::uint8_t, 4> bytes;
		read_bytes(bytes);
		return static_cast<std::uint32_t>(bytes[0]) << 24 | static_cast<std::uint32_t>(bytes[1]) << 16 |
		       static_cast<std::uint32_t>(bytes[2]) << 8 | static_cast<std::uint32_t>(bytes[3]);
	}

	std::uint32_t BinaryReader::read_little_endian_uint32()
	{
		std::array<std::uint8_t, 4> bytes;
		read_bytes(bytes);
		return static_cast<std::uint32_t>(bytes[3]) << 24 | static_cast<std::uint32_t>(bytes[2]) << 16 |
		       static_cast<std::uint32_t>(bytes[1]) << 8 | static_cast<std::uint32_t>(bytes[0]);
	}

	std::uint64_t BinaryReader::read_big_endian_uint64()
	{
		std::array<std::uint8_t, 8> bytes;
		read_bytes(bytes);
		return static_cast<std::uint64_t>(bytes[0]) << 56 | static_cast<std::uint64_t>(bytes[1]) << 48 |
		       static_cast<std::uint64_t>(bytes[2]) << 40 | static_cast<std::uint64_t>(bytes[3]) << 32 |
		       static_cast<std::uint64_t>(bytes[4]) << 24 | static_cast<std::uint64_t>(bytes[5]) << 16 |
		       static_cast<std::uint64_t>(bytes[6]) << 8 | static_cast<std::uint64_t>(bytes[7]);
	}

	std::uint64_t BinaryReader::read_little_endian_uint64()
	{
		std::array<std::uint8_t, 8> bytes;
		read_bytes(bytes);
		return static_cast<std::uint64_t>(bytes[7]) << 56 | static_cast<std::uint64_t>(bytes[6]) << 48 |
		       static_cast<std::uint64_t>(bytes[5]) << 40 | static_cast<std::uint64_t>(bytes[4]) << 32 |
		       static_cast<std::uint64_t>(bytes[3]) << 24 | static_cast<std::uint64_t>(bytes[2]) << 16 |
		       static_cast<std::uint64_t>(bytes[1]) << 8 | static_cast<std::uint64_t>(bytes[0]);
	}

	void BinaryReader::skip_bytes(std::size_t num_bytes)
	{
		if (num_bytes == 0)
			return;

		istream_->seekg(static_cast<std::istream::off_type>(num_bytes), std::ios::cur);

		if (!istream_->good())
			throw Exception(position(), "failed to skip " + std::to_string(num_bytes) + " bytes");
	}

	std::optional<std::int8_t> BinaryReader::peek_int8()
	{
		auto c = istream_->peek();

		if (istream_->eof())
			return std::nullopt;

		if (!istream_->good())
			throw Exception(position(), "failed to peek");

		return c;
	}

	std::string BinaryReader::position() const
	{
		if (!istream_)
			return "";

		std::string str;

		if (!position_prefix_.empty())
			str += position_prefix_;

		std::istream::pos_type pos = istream_->tellg();
		if (pos != std::istream::pos_type(-1))
		{
			if (!str.empty())
				str += ":";
			str += "<offset " + std::to_string(pos) + ">";
		}

		return str;
	}
}
