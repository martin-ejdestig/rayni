// This file is part of Rayni.
//
// Copyright (C) 2018-2020 Martin Ejdestig <marejde@gmail.com>
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

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace
{
	std::string position(const std::string &prefix, unsigned int offset)
	{
		std::string str;
		if (!prefix.empty())
			str += prefix + ":";
		str += "<offset " + std::to_string(offset) + ">";
		return str;
	}

	std::string position(unsigned int offset)
	{
		return position("", offset);
	}
}

namespace Rayni
{
	TEST(BinaryReader, OpenFile)
	{
		ScopedTempDir temp_dir;
		const std::string exists1_path = temp_dir.path() / "exists1";
		const std::string exists2_path = temp_dir.path() / "exists2";
		const std::string does_not_exist_path = temp_dir.path() / "does_not_exist";

		file_write(exists1_path, {0});
		file_write(exists2_path, {0});

		BinaryReader reader;
		EXPECT_EQ("", reader.position());

		reader.open_file(exists1_path);
		EXPECT_EQ(position(exists1_path, 0), reader.position());

		reader.open_file(exists2_path);
		EXPECT_EQ(position(exists2_path, 0), reader.position());

		EXPECT_THROW(reader.open_file(does_not_exist_path), BinaryReader::Exception);
	}

	TEST(BinaryReader, SetData)
	{
		BinaryReader reader;
		EXPECT_EQ("", reader.position());

		reader.set_data({0}, "prefix1");
		EXPECT_EQ(position("prefix1", 0), reader.position());

		reader.set_data({0}, "prefix2");
		EXPECT_EQ(position("prefix2", 0), reader.position());

		reader.set_data({0});
		EXPECT_EQ(position(0), reader.position());
	}

	TEST(BinaryReader, Close)
	{
		BinaryReader reader;
		reader.set_data({0}, "prefix");
		reader.close();
		EXPECT_EQ("", reader.position());
	}

	TEST(BinaryReader, ReadBytesToContainer)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 5, 6, 7, 8});

		std::array<std::uint8_t, 2> dest_array;
		reader.read_bytes(dest_array);
		const std::array<std::uint8_t, 2> expected_array = {1, 2};
		EXPECT_EQ(expected_array, dest_array);
		EXPECT_EQ(position(2), reader.position());

		std::vector<std::uint8_t> dest_vector(5);
		reader.read_bytes(dest_vector);
		EXPECT_EQ(std::vector<std::uint8_t>({3, 4, 5, 6, 7}), dest_vector);
		EXPECT_EQ(position(7), reader.position());

		std::array<std::uint8_t, 1> single_byte_array;
		reader.read_bytes(single_byte_array);
		EXPECT_EQ(8, single_byte_array[0]);
		EXPECT_EQ(position(8), reader.position());
		EXPECT_THROW(reader.read_bytes(single_byte_array), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBytesToEmptyContainer)
	{
		BinaryReader reader;
		reader.set_data({1, 2});

		std::vector<std::uint8_t> dest;

		EXPECT_THROW(reader.read_bytes(dest), BinaryReader::Exception);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(2);
		reader.read_bytes(dest);
		EXPECT_EQ(std::vector<std::uint8_t>({1, 2}), dest);
		EXPECT_EQ(position(2), reader.position());
	}

	TEST(BinaryReader, ReadBytesToContainerAndLimitNumBytes)
	{
		BinaryReader reader;
		reader.set_data({0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf0});

		const std::vector<std::uint8_t> orig = {1, 2, 3, 4};
		std::vector<std::uint8_t> dest;

		dest = orig;
		reader.read_bytes(dest, 0);
		EXPECT_EQ(orig, dest);
		EXPECT_EQ(position(0), reader.position());

		reader.read_bytes(dest, 1);
		EXPECT_EQ(std::vector<std::uint8_t>({0xff, 2, 3, 4}), dest);
		EXPECT_EQ(position(1), reader.position());

		reader.read_bytes(dest, 2);
		EXPECT_EQ(std::vector<std::uint8_t>({0xfe, 0xfd, 3, 4}), dest);
		EXPECT_EQ(position(3), reader.position());

		reader.read_bytes(dest, 4);
		EXPECT_EQ(std::vector<std::uint8_t>({0xfc, 0xfb, 0xfa, 0xf0}), dest);
		EXPECT_EQ(position(7), reader.position());

		EXPECT_THROW(reader.read_bytes(dest, 1), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBytesToContainerAndNumBytesTooLarge)
	{
		BinaryReader reader;
		reader.set_data({0x12, 0x34, 0x56, 0x78});

		static constexpr std::uint8_t UNTOUCHED_BYTE = 0xf0;
		std::vector<std::uint8_t> dest;

		dest.resize(1, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(1, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(2, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 4), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(2, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(3, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 4), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 5), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(3, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(4);
		reader.read_bytes(dest);
		EXPECT_EQ(std::vector<std::uint8_t>({0x12, 0x34, 0x56, 0x78}), dest);
		EXPECT_EQ(position(4), reader.position());
	}

	TEST(BinaryReader, ReadBytesToContainerAtOffset)
	{
		BinaryReader reader;
		reader.set_data({0x10, 0x20, 0x30, 0x40, 0x50, 0x60});

		std::vector<std::uint8_t> dest = {1, 2, 3, 4, 5, 6, 7, 8, 9};

		reader.read_bytes(dest, 1, 1);
		EXPECT_EQ(std::vector<std::uint8_t>({1, 0x10, 3, 4, 5, 6, 7, 8, 9}), dest);

		reader.read_bytes(dest, 3, 2);
		EXPECT_EQ(std::vector<std::uint8_t>({1, 0x10, 3, 0x20, 0x30, 6, 7, 8, 9}), dest);

		reader.read_bytes(dest, 6, 3);
		EXPECT_EQ(std::vector<std::uint8_t>({1, 0x10, 3, 0x20, 0x30, 6, 0x40, 0x50, 0x60}), dest);

		EXPECT_THROW(reader.read_bytes(dest, 2, 1), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>({1, 0x10, 3, 0x20, 0x30, 6, 0x40, 0x50, 0x60}), dest);
	}

	TEST(BinaryReader, ReadBytesToContainerAtOffsetOutOfRange)
	{
		BinaryReader reader;
		reader.set_data({0x12, 0x34, 0x56});

		static constexpr std::uint8_t UNTOUCHED_BYTE = 0xf0;
		std::vector<std::uint8_t> dest;

		dest.resize(1, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 1, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 1), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 1), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(1, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(2, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 2, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 1), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 1), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 2), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(2, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(3, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 3, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 1), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 4, 0), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 4, 1), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 4, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 4, 3), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(3, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		reader.read_bytes(dest);
		EXPECT_EQ(std::vector<std::uint8_t>({0x12, 0x34, 0x56}), dest);
		EXPECT_EQ(position(3), reader.position());
	}

	TEST(BinaryReader, ReadBytesToContainerAtOffsetNumBytesTooLarge)
	{
		BinaryReader reader;
		reader.set_data({0x12, 0x34, 0x56, 0x78});

		static constexpr std::uint8_t UNTOUCHED_BYTE = 0xf0;
		std::vector<std::uint8_t> dest;

		dest.resize(2, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 1, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 4), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(2, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(3, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 1, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 4), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 5), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 4), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(3, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		dest.resize(4, UNTOUCHED_BYTE);
		EXPECT_THROW(reader.read_bytes(dest, 1, 4), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 5), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 1, 6), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 4), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 2, 5), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 2), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 3), BinaryReader::Exception);
		EXPECT_THROW(reader.read_bytes(dest, 3, 4), BinaryReader::Exception);
		EXPECT_EQ(std::vector<std::uint8_t>(4, UNTOUCHED_BYTE), dest);
		EXPECT_EQ(position(0), reader.position());

		reader.read_bytes(dest);
		EXPECT_EQ(std::vector<std::uint8_t>({0x12, 0x34, 0x56, 0x78}), dest);
		EXPECT_EQ(position(4), reader.position());
	}

	TEST(BinaryReader, ReadInt8)
	{
		BinaryReader reader;
		reader.set_data({1, 0xff, 2, 0xfe, 3, 0xfd});

		EXPECT_EQ(1, reader.read_int8());
		EXPECT_EQ(position(1), reader.position());
		EXPECT_EQ(-1, reader.read_int8());
		EXPECT_EQ(position(2), reader.position());

		EXPECT_EQ(2, reader.read_big_endian<std::int8_t>());
		EXPECT_EQ(position(3), reader.position());
		EXPECT_EQ(-2, reader.read_big_endian<std::int8_t>());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(3, reader.read_little_endian<std::int8_t>());
		EXPECT_EQ(position(5), reader.position());
		EXPECT_EQ(-3, reader.read_little_endian<std::int8_t>());
		EXPECT_EQ(position(6), reader.position());

		EXPECT_THROW(reader.read_int8(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadUInt8)
	{
		BinaryReader reader;
		reader.set_data({1, 0xff, 2, 0xfe, 3, 0xfd});

		EXPECT_EQ(1, reader.read_uint8());
		EXPECT_EQ(position(1), reader.position());
		EXPECT_EQ(255, reader.read_uint8());
		EXPECT_EQ(position(2), reader.position());

		EXPECT_EQ(2, reader.read_big_endian<std::uint8_t>());
		EXPECT_EQ(position(3), reader.position());
		EXPECT_EQ(254, reader.read_big_endian<std::uint8_t>());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(3, reader.read_little_endian<std::uint8_t>());
		EXPECT_EQ(position(5), reader.position());
		EXPECT_EQ(253, reader.read_little_endian<std::uint8_t>());
		EXPECT_EQ(position(6), reader.position());

		EXPECT_THROW(reader.read_uint8(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianInt16)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 0xff, 0xfe, 3, 4, 0xff, 0xfd});

		EXPECT_EQ(0x0102, reader.read_big_endian_int16());
		EXPECT_EQ(position(2), reader.position());
		EXPECT_EQ(-2, reader.read_big_endian_int16());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(0x0304, reader.read_big_endian<std::int16_t>());
		EXPECT_EQ(position(6), reader.position());
		EXPECT_EQ(-3, reader.read_big_endian<std::int16_t>());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_THROW(reader.read_big_endian_int16(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianUInt16)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 0xff, 0xfe, 3, 4, 0xff, 0xfd});

		EXPECT_EQ(0x0102, reader.read_big_endian_uint16());
		EXPECT_EQ(position(2), reader.position());
		EXPECT_EQ(0xfffe, reader.read_big_endian_uint16());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(0x0304, reader.read_big_endian<std::uint16_t>());
		EXPECT_EQ(position(6), reader.position());
		EXPECT_EQ(0xfffd, reader.read_big_endian<std::uint16_t>());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_THROW(reader.read_big_endian_uint16(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianInt16)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 0xfe, 0xff, 3, 4, 0xfd, 0xff});

		EXPECT_EQ(0x0201, reader.read_little_endian_int16());
		EXPECT_EQ(position(2), reader.position());
		EXPECT_EQ(-2, reader.read_little_endian_int16());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(0x0403, reader.read_little_endian<std::int16_t>());
		EXPECT_EQ(position(6), reader.position());
		EXPECT_EQ(-3, reader.read_little_endian<std::int16_t>());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_THROW(reader.read_little_endian_int16(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianUInt16)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 0xfe, 0xff, 3, 4, 0xfd, 0xff});

		EXPECT_EQ(0x0201, reader.read_little_endian_uint16());
		EXPECT_EQ(position(2), reader.position());
		EXPECT_EQ(0xfffe, reader.read_little_endian_uint16());
		EXPECT_EQ(position(4), reader.position());

		EXPECT_EQ(0x0403, reader.read_little_endian<std::uint16_t>());
		EXPECT_EQ(position(6), reader.position());
		EXPECT_EQ(0xfffd, reader.read_little_endian<std::uint16_t>());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_THROW(reader.read_little_endian_uint16(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianInt32)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 0xff, 0xff, 0xff, 0xfe, 5, 6, 7, 8, 0xff, 0xff, 0xff, 0xfd});

		EXPECT_EQ(0x01020304, reader.read_big_endian_int32());
		EXPECT_EQ(position(4), reader.position());
		EXPECT_EQ(-2, reader.read_big_endian_int32());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_EQ(0x05060708, reader.read_big_endian<std::int32_t>());
		EXPECT_EQ(position(12), reader.position());
		EXPECT_EQ(-3, reader.read_big_endian<std::int32_t>());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_THROW(reader.read_big_endian_int32(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianUInt32)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 0xff, 0xff, 0xff, 0xfe, 5, 6, 7, 8, 0xff, 0xff, 0xff, 0xfd});

		EXPECT_EQ(0x01020304, reader.read_big_endian_uint32());
		EXPECT_EQ(position(4), reader.position());
		EXPECT_EQ(0xfffffffe, reader.read_big_endian_uint32());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_EQ(0x05060708, reader.read_big_endian<std::uint32_t>());
		EXPECT_EQ(position(12), reader.position());
		EXPECT_EQ(0xfffffffd, reader.read_big_endian<std::uint32_t>());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_THROW(reader.read_big_endian_uint32(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianInt32)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 0xfe, 0xff, 0xff, 0xff, 5, 6, 7, 8, 0xfd, 0xff, 0xff, 0xff});

		EXPECT_EQ(0x04030201, reader.read_little_endian_int32());
		EXPECT_EQ(position(4), reader.position());
		EXPECT_EQ(-2, reader.read_little_endian_int32());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_EQ(0x08070605, reader.read_little_endian<std::int32_t>());
		EXPECT_EQ(position(12), reader.position());
		EXPECT_EQ(-3, reader.read_little_endian<std::int32_t>());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_THROW(reader.read_little_endian_int32(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianUInt32)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 0xfe, 0xff, 0xff, 0xff, 5, 6, 7, 8, 0xfd, 0xff, 0xff, 0xff});

		EXPECT_EQ(0x04030201, reader.read_little_endian_uint32());
		EXPECT_EQ(position(4), reader.position());
		EXPECT_EQ(0xfffffffe, reader.read_little_endian_uint32());
		EXPECT_EQ(position(8), reader.position());

		EXPECT_EQ(0x08070605, reader.read_little_endian<std::uint32_t>());
		EXPECT_EQ(position(12), reader.position());
		EXPECT_EQ(0xfffffffd, reader.read_little_endian<std::uint32_t>());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_THROW(reader.read_little_endian_uint32(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianInt64)
	{
		BinaryReader reader;
		reader.set_data({1, 2,  3,  4,  5,  6,  7,  8,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
		                 9, 10, 11, 12, 13, 14, 15, 16, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd});

		EXPECT_EQ(0x0102030405060708, reader.read_big_endian_int64());
		EXPECT_EQ(position(8), reader.position());
		EXPECT_EQ(-2, reader.read_big_endian_int64());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_EQ(0x090a0b0c0d0e0f10, reader.read_big_endian<std::int64_t>());
		EXPECT_EQ(position(24), reader.position());
		EXPECT_EQ(-3, reader.read_big_endian<std::int64_t>());
		EXPECT_EQ(position(32), reader.position());

		EXPECT_THROW(reader.read_big_endian_int64(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianUInt64)
	{
		BinaryReader reader;
		reader.set_data({1, 2,  3,  4,  5,  6,  7,  8,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
		                 9, 10, 11, 12, 13, 14, 15, 16, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd});

		EXPECT_EQ(0x0102030405060708, reader.read_big_endian_uint64());
		EXPECT_EQ(position(8), reader.position());
		EXPECT_EQ(0xfffffffffffffffe, reader.read_big_endian_uint64());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_EQ(0x090a0b0c0d0e0f10, reader.read_big_endian<std::uint64_t>());
		EXPECT_EQ(position(24), reader.position());
		EXPECT_EQ(0xfffffffffffffffd, reader.read_big_endian<std::uint64_t>());
		EXPECT_EQ(position(32), reader.position());

		EXPECT_THROW(reader.read_big_endian_uint64(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianInt64)
	{
		BinaryReader reader;
		reader.set_data({1, 2,  3,  4,  5,  6,  7,  8,  0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		                 9, 10, 11, 12, 13, 14, 15, 16, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});

		EXPECT_EQ(0x0807060504030201, reader.read_little_endian_int64());
		EXPECT_EQ(position(8), reader.position());
		EXPECT_EQ(-2, reader.read_little_endian_int64());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_EQ(0x100f0e0d0c0b0a09, reader.read_little_endian<std::int64_t>());
		EXPECT_EQ(position(24), reader.position());
		EXPECT_EQ(-3, reader.read_little_endian<std::int64_t>());
		EXPECT_EQ(position(32), reader.position());

		EXPECT_THROW(reader.read_little_endian_int64(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianUInt64)
	{
		BinaryReader reader;
		reader.set_data({1, 2,  3,  4,  5,  6,  7,  8,  0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		                 9, 10, 11, 12, 13, 14, 15, 16, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff});

		EXPECT_EQ(0x0807060504030201, reader.read_little_endian_uint64());
		EXPECT_EQ(position(8), reader.position());
		EXPECT_EQ(0xfffffffffffffffe, reader.read_little_endian_uint64());
		EXPECT_EQ(position(16), reader.position());

		EXPECT_EQ(0x100f0e0d0c0b0a09, reader.read_little_endian<std::uint64_t>());
		EXPECT_EQ(position(24), reader.position());
		EXPECT_EQ(0xfffffffffffffffd, reader.read_little_endian<std::uint64_t>());
		EXPECT_EQ(position(32), reader.position());

		EXPECT_THROW(reader.read_little_endian_uint64(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianIEEE754Float)
	{
		BinaryReader reader;
		reader.set_data({0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0xbf, 0x80, 0x00, 0x00, 0x44, 0x9a,
		                 0x52, 0x2c, 0xc4, 0x9a, 0x52, 0x2c, 0x44, 0x9a, 0x52, 0x2c, 0xc4, 0x9a, 0x52, 0x2c});

		EXPECT_NEAR(0, reader.read_big_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(4), reader.position());

		EXPECT_NEAR(1.0, reader.read_big_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(8), reader.position());
		EXPECT_NEAR(-1.0, reader.read_big_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(12), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_big_endian_ieee_754_float(), 1e-4);
		EXPECT_EQ(position(16), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_big_endian_ieee_754_float(), 1e-4);
		EXPECT_EQ(position(20), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_big_endian<float>(), 1e-4);
		EXPECT_EQ(position(24), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_big_endian<float>(), 1e-4);
		EXPECT_EQ(position(28), reader.position());

		EXPECT_THROW(reader.read_big_endian_ieee_754_float(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianIEEE754Float)
	{
		BinaryReader reader;
		reader.set_data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0xbf, 0x2c, 0x52,
		                 0x9a, 0x44, 0x2c, 0x52, 0x9a, 0xc4, 0x2c, 0x52, 0x9a, 0x44, 0x2c, 0x52, 0x9a, 0xc4});

		EXPECT_NEAR(0, reader.read_little_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(4), reader.position());

		EXPECT_NEAR(1.0, reader.read_little_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(8), reader.position());
		EXPECT_NEAR(-1.0, reader.read_little_endian_ieee_754_float(), 1e-100);
		EXPECT_EQ(position(12), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_little_endian_ieee_754_float(), 1e-4);
		EXPECT_EQ(position(16), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_little_endian_ieee_754_float(), 1e-4);
		EXPECT_EQ(position(20), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_little_endian<float>(), 1e-4);
		EXPECT_EQ(position(24), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_little_endian<float>(), 1e-4);
		EXPECT_EQ(position(28), reader.position());

		EXPECT_THROW(reader.read_little_endian_ieee_754_float(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianIEEE754Double)
	{
		BinaryReader reader;
		reader.set_data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00,
		                 0x00, 0x00, 0xbf, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x93, 0x4a, 0x45,
		                 0x84, 0xf4, 0xc6, 0xe7, 0xc0, 0x93, 0x4a, 0x45, 0x84, 0xf4, 0xc6, 0xe7, 0x40, 0x93,
		                 0x4a, 0x45, 0x84, 0xf4, 0xc6, 0xe7, 0xc0, 0x93, 0x4a, 0x45, 0x84, 0xf4, 0xc6, 0xe7});

		EXPECT_NEAR(0, reader.read_big_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(8), reader.position());

		EXPECT_NEAR(1.0, reader.read_big_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(16), reader.position());
		EXPECT_NEAR(-1.0, reader.read_big_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(24), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_big_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(32), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_big_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(40), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_big_endian<double>(), 1e-100);
		EXPECT_EQ(position(48), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_big_endian<double>(), 1e-100);
		EXPECT_EQ(position(56), reader.position());

		EXPECT_THROW(reader.read_big_endian_ieee_754_double(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianIEEE754Double)
	{
		BinaryReader reader;
		reader.set_data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		                 0xf0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xbf, 0xe7, 0xc6, 0xf4, 0x84,
		                 0x45, 0x4a, 0x93, 0x40, 0xe7, 0xc6, 0xf4, 0x84, 0x45, 0x4a, 0x93, 0xc0, 0xe7, 0xc6,
		                 0xf4, 0x84, 0x45, 0x4a, 0x93, 0x40, 0xe7, 0xc6, 0xf4, 0x84, 0x45, 0x4a, 0x93, 0xc0});

		EXPECT_NEAR(0, reader.read_little_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(8), reader.position());

		EXPECT_NEAR(1.0, reader.read_little_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(16), reader.position());
		EXPECT_NEAR(-1.0, reader.read_little_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(24), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_little_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(32), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_little_endian_ieee_754_double(), 1e-100);
		EXPECT_EQ(position(40), reader.position());

		EXPECT_NEAR(1234.56789, reader.read_little_endian<double>(), 1e-100);
		EXPECT_EQ(position(48), reader.position());
		EXPECT_NEAR(-1234.56789, reader.read_little_endian<double>(), 1e-100);
		EXPECT_EQ(position(56), reader.position());

		EXPECT_THROW(reader.read_little_endian_ieee_754_double(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadBigEndianUnsopportedTypeThrows)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

		EXPECT_THROW(reader.read_big_endian<bool>(), BinaryReader::Exception);
		EXPECT_THROW(reader.read_big_endian<void *>(), BinaryReader::Exception);
	}

	TEST(BinaryReader, ReadLittleEndianUnsopportedTypeThrows)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

		EXPECT_THROW(reader.read_little_endian<bool>(), BinaryReader::Exception);
		EXPECT_THROW(reader.read_little_endian<void *>(), BinaryReader::Exception);
	}

	TEST(BinaryReader, SkipBytes)
	{
		BinaryReader reader;
		reader.set_data({1, 2, 3, 4, 5, 6});

		reader.skip_bytes(0);
		EXPECT_EQ(position(0), reader.position());

		reader.skip_bytes(1);
		EXPECT_EQ(position(1), reader.position());

		reader.skip_bytes(0);
		EXPECT_EQ(position(1), reader.position());

		reader.skip_bytes(2);
		EXPECT_EQ(position(3), reader.position());

		reader.skip_bytes(3);
		EXPECT_EQ(position(6), reader.position());

		reader.skip_bytes(0);
		EXPECT_EQ(position(6), reader.position());

		EXPECT_THROW(reader.skip_bytes(1), BinaryReader::Exception);

		reader.set_data({1, 2});
		reader.skip_bytes(2);
		EXPECT_EQ(position(2), reader.position());

		reader.set_data({1, 2});
		EXPECT_THROW(reader.skip_bytes(3), BinaryReader::Exception);

		reader.set_data({1, 2});
		EXPECT_THROW(reader.skip_bytes(4), BinaryReader::Exception);
	}

	TEST(BinaryReader, PeekInt8)
	{
		BinaryReader reader;
		reader.set_data({1, 2});

		auto b1 = reader.peek_int8();
		EXPECT_EQ(1, b1.value());
		EXPECT_EQ(position(0), reader.position());

		auto b2 = reader.peek_int8();
		EXPECT_EQ(1, b2.value());
		EXPECT_EQ(position(0), reader.position());

		reader.skip_bytes(1);

		auto b3 = reader.peek_int8();
		EXPECT_EQ(2, b3.value());
		EXPECT_EQ(position(1), reader.position());

		auto b4 = reader.peek_int8();
		EXPECT_EQ(2, b4.value());
		EXPECT_EQ(position(1), reader.position());

		reader.skip_bytes(1);

		auto b5 = reader.peek_int8();
		EXPECT_FALSE(b5.has_value());

		auto b6 = reader.peek_int8();
		EXPECT_FALSE(b6.has_value());
	}

	TEST(BinaryReader, PositionAtEOF)
	{
		BinaryReader reader;

		reader.set_data({0});
		EXPECT_EQ(0, reader.read_int8());
		EXPECT_EQ(position(1), reader.position());
		EXPECT_THROW(reader.read_int8(), BinaryReader::Exception);
		EXPECT_EQ(position(1), reader.position());

		reader.set_data({0}, "prefix");
		EXPECT_EQ(0, reader.read_int8());
		EXPECT_EQ(position("prefix", 1), reader.position());
		EXPECT_THROW(reader.read_int8(), BinaryReader::Exception);
		EXPECT_EQ(position("prefix", 1), reader.position());
	}
}
