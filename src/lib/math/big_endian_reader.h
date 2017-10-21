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

#ifndef RAYNI_LIB_MATH_BIG_ENDIAN_READER_H
#define RAYNI_LIB_MATH_BIG_ENDIAN_READER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "lib/math/math.h"

namespace Rayni
{
	template <typename Byte>
	class BigEndianReader
	{
	public:
		using Container = std::vector<Byte>;
		using Size = typename Container::size_type;

		static_assert(sizeof(Byte) == 1, "size of byte type must 1");

		explicit BigEndianReader(const Container &data) : data_(data)
		{
		}

		Size bytes_left() const
		{
			return data_.size() - position_;
		}

		std::int8_t read_int8()
		{
			return static_cast<std::int8_t>(read_uint8());
		}

		std::uint8_t read_uint8()
		{
			auto b = static_cast<std::uint8_t>(data_.at(position_));
			position_++;
			return b;
		}

		std::int16_t read_int16()
		{
			return static_cast<std::int16_t>(read_uint16());
		}

		std::uint16_t read_uint16()
		{
			std::uint8_t b1 = read_uint8();
			std::uint8_t b2 = read_uint8();
			return b1 << 8 | b2;
		}

		std::int32_t read_int32()
		{
			return static_cast<std::int32_t>(read_uint32());
		}

		std::uint32_t read_uint32()
		{
			std::uint16_t b1b2 = read_uint16();
			std::uint16_t b3b4 = read_uint16();
			return static_cast<std::uint32_t>(b1b2 << 16 | b3b4);
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::int8_t>::value, Int> read()
		{
			return read_int8();
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::uint8_t>::value, Int> read()
		{
			return read_uint8();
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::int16_t>::value, Int> read()
		{
			return read_int16();
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::uint16_t>::value, Int> read()
		{
			return read_uint16();
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::int32_t>::value, Int> read()
		{
			return read_int32();
		}

		template <typename Int>
		std::enable_if_t<std::is_same<Int, std::uint32_t>::value, Int> read()
		{
			return read_uint32();
		}

		template <typename Int>
		real_t read_fixed_point(unsigned int denominator)
		{
			return real_t(read<Int>()) / denominator;
		}

		template <typename Int, std::size_t NUM_INTS>
		std::array<Int, NUM_INTS> read()
		{
			std::array<Int, NUM_INTS> values;

			for (Int &value : values)
				value = read<Int>();

			return values;
		}

		template <typename Int, std::size_t NUM_INTS>
		std::array<real_t, NUM_INTS> read_fixed_point(unsigned int denominator)
		{
			std::array<real_t, NUM_INTS> values;

			for (real_t &value : values)
				value = read_fixed_point<Int>(denominator);

			return values;
		}

		template <typename Value, typename Int, std::size_t NUM_INTS>
		std::enable_if_t<std::is_constructible<Value, const std::array<Int, NUM_INTS> &>::value,
		                 std::vector<Value>>
		read(std::size_t number_of_values)
		{
			std::vector<Value> values;

			for (std::size_t i = 0; i < number_of_values; i++)
				values.emplace_back(read<Int, NUM_INTS>());

			return values;
		}

		template <typename Value, typename Int, std::size_t NUM_INTS>
		std::enable_if_t<std::is_constructible<Value, const std::array<Int, NUM_INTS> &>::value,
		                 std::vector<Value>>
		read()
		{
			return read<Value, Int, NUM_INTS>(bytes_left() / (sizeof(Int) * NUM_INTS));
		}

		template <typename Value, typename Int, std::size_t NUM_INTS>
		std::enable_if_t<std::is_constructible<Value, const std::array<real_t, NUM_INTS> &>::value,
		                 std::vector<Value>>
		read_fixed_point(unsigned int denominator, std::size_t number_of_values)
		{
			std::vector<Value> values;

			for (std::size_t i = 0; i < number_of_values; i++)
				values.emplace_back(read_fixed_point<Int, NUM_INTS>(denominator));

			return values;
		}

		template <typename Value, typename Int, std::size_t NUM_INTS>
		std::enable_if_t<std::is_constructible<Value, const std::array<real_t, NUM_INTS> &>::value,
		                 std::vector<Value>>
		read_fixed_point(unsigned int denominator)
		{
			return read_fixed_point<Value, Int, NUM_INTS>(denominator,
			                                              bytes_left() / (sizeof(Int) * NUM_INTS));
		}

	private:
		const Container &data_;
		Size position_ = 0;
	};
}

#endif // RAYNI_LIB_MATH_BIG_ENDIAN_READER_H
