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

#ifndef RAYNI_LIB_IO_BINARY_READER_H
#define RAYNI_LIB_IO_BINARY_READER_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	class BinaryReader
	{
	public:
		class Exception : public std::runtime_error
		{
		public:
			using std::runtime_error::runtime_error;

			Exception(const std::string &prefix, const std::string &str) :
			        std::runtime_error(prefix.empty() ? str : prefix + ": " + str)
			{
			}
		};

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

		std::uint8_t read_uint8()
		{
			const std::uint8_t *p = next_bytes(1);

			return *p;
		}

		std::int8_t read_int8()
		{
			return static_cast<std::int8_t>(read_uint8());
		}

		std::uint16_t read_big_endian_uint16()
		{
			const std::uint8_t *p = next_bytes(2);

			return static_cast<std::uint16_t>(p[0]) << 8 | static_cast<std::uint16_t>(p[1]);
		}

		std::int16_t read_big_endian_int16()
		{
			return static_cast<std::int16_t>(read_big_endian_uint16());
		}

		std::uint16_t read_little_endian_uint16()
		{
			const std::uint8_t *p = next_bytes(2);

			return static_cast<std::uint16_t>(p[1]) << 8 | static_cast<std::uint16_t>(p[0]);
		}

		std::int16_t read_little_endian_int16()
		{
			return static_cast<std::int16_t>(read_little_endian_uint16());
		}

		std::uint32_t read_big_endian_uint32()
		{
			const std::uint8_t *p = next_bytes(4);

			return static_cast<std::uint32_t>(p[0]) << 24 | static_cast<std::uint32_t>(p[1]) << 16 |
			       static_cast<std::uint32_t>(p[2]) << 8 | static_cast<std::uint32_t>(p[3]);
		}

		std::int32_t read_big_endian_int32()
		{
			return static_cast<std::int32_t>(read_big_endian_uint32());
		}

		std::uint32_t read_little_endian_uint32()
		{
			const std::uint8_t *p = next_bytes(4);

			return static_cast<std::uint32_t>(p[3]) << 24 | static_cast<std::uint32_t>(p[2]) << 16 |
			       static_cast<std::uint32_t>(p[1]) << 8 | static_cast<std::uint32_t>(p[0]);
		}

		std::int32_t read_little_endian_int32()
		{
			return static_cast<std::int32_t>(read_little_endian_uint32());
		}

		std::uint64_t read_big_endian_uint64()
		{
			const std::uint8_t *p = next_bytes(8);

			return static_cast<std::uint64_t>(p[0]) << 56 | static_cast<std::uint64_t>(p[1]) << 48 |
			       static_cast<std::uint64_t>(p[2]) << 40 | static_cast<std::uint64_t>(p[3]) << 32 |
			       static_cast<std::uint64_t>(p[4]) << 24 | static_cast<std::uint64_t>(p[5]) << 16 |
			       static_cast<std::uint64_t>(p[6]) << 8 | static_cast<std::uint64_t>(p[7]);
		}

		std::int64_t read_big_endian_int64()
		{
			return static_cast<std::int64_t>(read_big_endian_uint64());
		}

		std::uint64_t read_little_endian_uint64()
		{
			const std::uint8_t *p = next_bytes(8);

			return static_cast<std::uint64_t>(p[7]) << 56 | static_cast<std::uint64_t>(p[6]) << 48 |
			       static_cast<std::uint64_t>(p[5]) << 40 | static_cast<std::uint64_t>(p[4]) << 32 |
			       static_cast<std::uint64_t>(p[3]) << 24 | static_cast<std::uint64_t>(p[2]) << 16 |
			       static_cast<std::uint64_t>(p[1]) << 8 | static_cast<std::uint64_t>(p[0]);
		}

		std::int64_t read_little_endian_int64()
		{
			return static_cast<std::int64_t>(read_little_endian_uint64());
		}

		float read_big_endian_ieee_754_float()
		{
			return float_from_int<float>(read_big_endian_uint32());
		}

		float read_little_endian_ieee_754_float()
		{
			return float_from_int<float>(read_little_endian_uint32());
		}

		double read_big_endian_ieee_754_double()
		{
			return float_from_int<double>(read_big_endian_uint64());
		}

		double read_little_endian_ieee_754_double()
		{
			return float_from_int<double>(read_little_endian_uint64());
		}

		template <typename T>
		T read_big_endian();

		template <typename T>
		T read_little_endian();

		void skip_bytes(std::size_t num_bytes);

		std::optional<std::int8_t> peek_int8()
		{
			if (buffer_position_ >= buffer_size_)
				return {};

			return buffer_[buffer_position_];
		}

		std::string position() const;

	private:
		void read_bytes(void *dest, std::size_t dest_size, std::size_t dest_offset, std::size_t num_bytes);

		const std::uint8_t *next_bytes(unsigned int i)
		{
			if (buffer_position_ + i > buffer_size_)
				throw Exception(position(), "unexpected end of stream");

			const std::uint8_t *p = buffer_ + buffer_position_;
			buffer_position_ += i;

			return p;
		}

		template <typename F, typename I>
		static F float_from_int(I i)
		{
			static_assert(std::is_integral_v<I> && std::is_floating_point_v<F> && sizeof(I) == sizeof(F));
			static_assert(std::numeric_limits<F>::is_iec559, "float is not IEEE 754");
			union
			{
				I i;
				F f;
			} u;
			u.i = i;
			return u.f;
		}

		MemoryMappedFile mmap_file_;
		std::vector<std::uint8_t> data_;

		const std::uint8_t *buffer_ = nullptr;
		std::size_t buffer_size_ = 0;
		std::size_t buffer_position_ = 0;

		std::string position_prefix_;
	};

	template <typename T>
	T BinaryReader::read_big_endian()
	{
		if constexpr (std::is_same_v<T, std::int8_t>)
			return read_int8();

		if constexpr (std::is_same_v<T, std::uint8_t>)
			return read_uint8();

		if constexpr (std::is_same_v<T, std::int16_t>)
			return read_big_endian_int16();

		if constexpr (std::is_same_v<T, std::uint16_t>)
			return read_big_endian_uint16();

		if constexpr (std::is_same_v<T, std::int32_t>)
			return read_big_endian_int32();

		if constexpr (std::is_same_v<T, std::uint32_t>)
			return read_big_endian_uint32();

		if constexpr (std::is_same_v<T, std::int64_t>)
			return read_big_endian_int64();

		if constexpr (std::is_same_v<T, std::uint64_t>)
			return read_big_endian_uint64();

		if constexpr (std::is_same_v<T, float>)
			return read_big_endian_ieee_754_float();

		if constexpr (std::is_same_v<T, double>)
			return read_big_endian_ieee_754_double();

		throw Exception("Invalid type for read_big_endian().");
	}

	template <typename T>
	T BinaryReader::read_little_endian()
	{
		if constexpr (std::is_same_v<T, std::int8_t>)
			return read_int8();

		if constexpr (std::is_same_v<T, std::uint8_t>)
			return read_uint8();

		if constexpr (std::is_same_v<T, std::int16_t>)
			return read_little_endian_int16();

		if constexpr (std::is_same_v<T, std::uint16_t>)
			return read_little_endian_uint16();

		if constexpr (std::is_same_v<T, std::int32_t>)
			return read_little_endian_int32();

		if constexpr (std::is_same_v<T, std::uint32_t>)
			return read_little_endian_uint32();

		if constexpr (std::is_same_v<T, std::int64_t>)
			return read_little_endian_int64();

		if constexpr (std::is_same_v<T, std::uint64_t>)
			return read_little_endian_uint64();

		if constexpr (std::is_same_v<T, float>)
			return read_little_endian_ieee_754_float();

		if constexpr (std::is_same_v<T, double>)
			return read_little_endian_ieee_754_double();

		throw Exception("Invalid type for BinaryReader::read_little_endian().");
	}
}

#endif // RAYNI_LIB_IO_BINARY_READER_H
