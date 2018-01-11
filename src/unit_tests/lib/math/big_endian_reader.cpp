/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2018 Martin Ejdestig <marejde@gmail.com>
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

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "lib/math/big_endian_reader.h"
#include "lib/math/math.h"

namespace Rayni
{
	TEST(BigEndianReader, Read8)
	{
		const std::vector<std::uint8_t> data = {0x12, 0x34, 0xfe, 0xfe, 0x80, 0x80};
		BigEndianReader<std::uint8_t> reader(data);

		ASSERT_EQ(6, reader.bytes_left());
		EXPECT_EQ(0x12, reader.read_int8());

		ASSERT_EQ(5, reader.bytes_left());
		EXPECT_EQ(0x34, reader.read_uint8());

		ASSERT_EQ(4, reader.bytes_left());
		EXPECT_EQ(-2, reader.read_int8());

		ASSERT_EQ(3, reader.bytes_left());
		EXPECT_EQ(0xfe, reader.read_uint8());

		ASSERT_EQ(2, reader.bytes_left());
		EXPECT_EQ(-0x80, reader.read<std::int8_t>());

		ASSERT_EQ(1, reader.bytes_left());
		EXPECT_EQ(0x80, reader.read<std::uint8_t>());

		ASSERT_EQ(0, reader.bytes_left());
		EXPECT_THROW(reader.read_int8(), std::out_of_range);
	}

	TEST(BigEndianReader, Read16)
	{
		const std::vector<std::uint8_t> data =
		        {0x12, 0x34, 0x56, 0x78, 0xfe, 0xdc, 0xfe, 0xdc, 0x80, 0x00, 0x80, 0x00};
		BigEndianReader<std::uint8_t> reader(data);

		ASSERT_EQ(12, reader.bytes_left());
		EXPECT_EQ(0x1234, reader.read_int16());

		ASSERT_EQ(10, reader.bytes_left());
		EXPECT_EQ(0x5678, reader.read_uint16());

		ASSERT_EQ(8, reader.bytes_left());
		EXPECT_EQ(-292, reader.read_int16());

		ASSERT_EQ(6, reader.bytes_left());
		EXPECT_EQ(0xfedc, reader.read_uint16());

		ASSERT_EQ(4, reader.bytes_left());
		EXPECT_EQ(-0x8000, reader.read<std::int16_t>());

		ASSERT_EQ(2, reader.bytes_left());
		EXPECT_EQ(0x8000, reader.read<std::uint16_t>());

		ASSERT_EQ(0, reader.bytes_left());
		EXPECT_THROW(reader.read_int16(), std::out_of_range);
	}

	TEST(BigEndianReader, Read32)
	{
		const std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78, 0x12, 0x34, 0x56, 0x78,
		                                        0xfe, 0xdc, 0xba, 0x98, 0xfe, 0xdc, 0xba, 0x98,
		                                        0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00};
		BigEndianReader<std::uint8_t> reader(data);

		ASSERT_EQ(24, reader.bytes_left());
		EXPECT_EQ(0x12345678, reader.read_int32());

		ASSERT_EQ(20, reader.bytes_left());
		EXPECT_EQ(0x12345678, reader.read_uint32());

		ASSERT_EQ(16, reader.bytes_left());
		EXPECT_EQ(-19088744, reader.read_int32());

		ASSERT_EQ(12, reader.bytes_left());
		EXPECT_EQ(0xfedcba98, reader.read_uint32());

		ASSERT_EQ(8, reader.bytes_left());
		EXPECT_EQ(-0x80000000, reader.read<std::int32_t>());

		ASSERT_EQ(4, reader.bytes_left());
		EXPECT_EQ(0x80000000, reader.read<std::uint32_t>());

		ASSERT_EQ(0, reader.bytes_left());
		EXPECT_THROW(reader.read_int32(), std::out_of_range);
	}

	TEST(BigEndianReader, ReadFixedPoint)
	{
		const std::vector<std::uint8_t> data = {0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0xff, 0xff,
		                                        0x00, 0x00, 0xff, 0xfe, 0x80, 0x00, 0xff, 0xff, 0x00, 0x00};
		BigEndianReader<std::uint8_t> reader(data);

		EXPECT_NEAR(1.0, reader.read_fixed_point<std::int32_t>(0x10000), 1e-100);
		EXPECT_NEAR(1.5, reader.read_fixed_point<std::int32_t>(0x10000), 1e-100);
		EXPECT_NEAR(-1.0, reader.read_fixed_point<std::int32_t>(0x10000), 1e-100);
		EXPECT_NEAR(-1.5, reader.read_fixed_point<std::int32_t>(0x10000), 1e-100);

		EXPECT_NEAR(65535.0, reader.read_fixed_point<std::uint32_t>(0x10000), 1e-100);
	}

	TEST(BigEndianReader, ReadArray)
	{
		const std::vector<std::uint8_t> data = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
		BigEndianReader<std::uint8_t> reader(data);

		std::array<std::uint16_t, 3> array = reader.read<std::uint16_t, 3>();
		EXPECT_EQ(0x1234, array[0]);
		EXPECT_EQ(0x5678, array[1]);
		EXPECT_EQ(0x9abc, array[2]);
	}

	TEST(BigEndianReader, ReadFixedPointArray)
	{
		const std::vector<std::uint8_t> data = {0x7f, 0xff, 0x80, 0x00, 0x40, 0x00};
		BigEndianReader<std::uint8_t> reader(data);

		std::array<real_t, 3> array = reader.read_fixed_point<std::int16_t, 3>(0x8000);
		EXPECT_NEAR(1.0, array[0], 3.06e-5);
		EXPECT_NEAR(-1.0, array[1], 1e-100);
		EXPECT_NEAR(0.5, array[2], 1e-100);
	}

	TEST(BigEndianReader, ReadVectorArrayConstructor)
	{
		struct Foo
		{
			explicit Foo(const std::array<std::int8_t, 2> &array) : x(array[0]), y(array[1])
			{
			}

			std::int8_t x = 0;
			std::int8_t y = 0;
		};

		const std::vector<std::uint8_t> data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		BigEndianReader<std::uint8_t> reader(data);

		std::vector<Foo> foos1 = reader.read<Foo, std::int8_t, 2>(2);
		EXPECT_EQ(5, reader.bytes_left());
		ASSERT_EQ(2, foos1.size());
		EXPECT_EQ(0x00, foos1[0].x);
		EXPECT_EQ(0x11, foos1[0].y);
		EXPECT_EQ(0x22, foos1[1].x);
		EXPECT_EQ(0x33, foos1[1].y);

		std::vector<Foo> foos2 = reader.read<Foo, std::int8_t, 2>(); // Read as many Foos as possible.
		EXPECT_EQ(1, reader.bytes_left()); // 1 trailing byte left since data.size() % 2 == 1
		ASSERT_EQ(2, foos2.size());
		EXPECT_EQ(0x44, foos2[0].x);
		EXPECT_EQ(0x55, foos2[0].y);
		EXPECT_EQ(0x66, foos2[1].x);
		EXPECT_EQ(0x77, foos2[1].y);
	}

	TEST(BigEndianReader, ReadFixedPointVectorArrayConstructor)
	{
		struct Foo
		{
			explicit Foo(const std::array<real_t, 2> &array) : x(array[0]), y(array[1])
			{
			}

			real_t x = 0;
			real_t y = 0;
		};

		const std::vector<std::uint8_t> data = {0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0, 0x00, 0xff};
		BigEndianReader<std::uint8_t> reader(data);

		std::vector<Foo> foos1 = reader.read_fixed_point<Foo, std::uint8_t, 2>(0x80, 2);
		EXPECT_EQ(5, reader.bytes_left());
		ASSERT_EQ(2, foos1.size());
		EXPECT_NEAR(0.25, foos1[0].x, 1e-100);
		EXPECT_NEAR(0.50, foos1[0].y, 1e-100);
		EXPECT_NEAR(0.75, foos1[1].x, 1e-100);
		EXPECT_NEAR(1.00, foos1[1].y, 1e-100);

		std::vector<Foo> foos2 =
		        reader.read_fixed_point<Foo, std::uint8_t, 2>(0x80); // Read as many Foos as possible.
		EXPECT_EQ(1, reader.bytes_left()); // 1 trailing byte left since data.size() % 2 == 1
		ASSERT_EQ(2, foos2.size());
		EXPECT_NEAR(1.25, foos2[0].x, 1e-100);
		EXPECT_NEAR(1.50, foos2[0].y, 1e-100);
		EXPECT_NEAR(1.75, foos2[1].x, 1e-100);
		EXPECT_NEAR(0.00, foos2[1].y, 1e-100);
	}
}
