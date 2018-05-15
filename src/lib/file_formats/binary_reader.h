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

#ifndef RAYNI_LIB_FILE_FORMATS_BINARY_READER_H
#define RAYNI_LIB_FILE_FORMATS_BINARY_READER_H

#include <cstddef>
#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lib/file_formats/file_format_exception.h"
#include "lib/function/scope_exit.h"

namespace Rayni
{
	class BinaryReader
	{
	public:
		class Exception;

		void open_file(const std::string &file_name);

		void set_data(std::vector<std::uint8_t> &&data, const std::string &position_prefix);
		void set_data(std::vector<std::uint8_t> &&data)
		{
			set_data(std::move(data), "");
		}

		void close();

		template <typename T>
		void read_bytes(T &dest)
		{
			read_bytes(dest, 0, dest.size());
		}

		template <typename T>
		void read_bytes(T &dest, std::size_t num_bytes)
		{
			read_bytes(dest, 0, num_bytes);
		}

		template <typename T>
		void read_bytes(T &dest, std::size_t dest_offset, std::size_t num_bytes)
		{
			static_assert(sizeof(typename T::value_type) == 1);
			read_bytes(dest.data(), dest.size(), dest_offset, num_bytes);
		}

		std::uint8_t read_uint8();
		std::int8_t read_int8()
		{
			return static_cast<std::int8_t>(read_uint8());
		}

		std::uint16_t read_big_endian_uint16();
		std::int16_t read_big_endian_int16()
		{
			return static_cast<std::int16_t>(read_big_endian_uint16());
		}

		std::uint16_t read_little_endian_uint16();
		std::int16_t read_little_endian_int16()
		{
			return static_cast<std::int16_t>(read_little_endian_uint16());
		}

		std::uint32_t read_big_endian_uint32();
		std::int32_t read_big_endian_int32()
		{
			return static_cast<std::int32_t>(read_big_endian_uint32());
		}

		std::uint32_t read_little_endian_uint32();
		std::int32_t read_little_endian_int32()
		{
			return static_cast<std::int32_t>(read_little_endian_uint32());
		}

		std::uint64_t read_big_endian_uint64();
		std::int64_t read_big_endian_int64()
		{
			return static_cast<std::int64_t>(read_big_endian_uint64());
		}

		std::uint64_t read_little_endian_uint64();
		std::int64_t read_little_endian_int64()
		{
			return static_cast<std::int64_t>(read_little_endian_uint64());
		}

		void skip_bytes(std::size_t num_bytes);

		std::optional<std::int8_t> peek_int8();

		std::string position() const;

	private:
		void reset(std::unique_ptr<std::istream> &&istream, const std::string &position_prefix);

		void read_bytes(void *dest, std::size_t dest_size, std::size_t dest_offset, std::size_t num_bytes);

		std::unique_ptr<std::istream> istream_;
		std::string position_prefix_;
	};

	class BinaryReader::Exception : public FileFormatException
	{
	public:
		using FileFormatException::FileFormatException;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_BINARY_READER_H
