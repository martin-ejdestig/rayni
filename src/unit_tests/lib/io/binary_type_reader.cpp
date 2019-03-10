// This file is part of Rayni.
//
// Copyright (C) 2018-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/io/binary_type_reader.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		struct Foo
		{
			std::uint16_t bar = 0;
			std::uint16_t baz = 0;
		};

		class FooReader : public BinaryTypeReader<Foo>
		{
		private:
			Foo read() override
			{
				Foo foo;

				foo.bar = read_big_endian_uint16();
				foo.baz = read_little_endian_uint16();

				return foo;
			}
		};
	}

	TEST(BinaryTypeReader, ReadFile)
	{
		ScopedTempDir temp_dir;
		const std::string good_path = temp_dir.path() / "good.bin";
		const std::string bad_path = temp_dir.path() / "bad.bin";
		const std::string does_not_exist_path = temp_dir.path() / "does_not_exist.bin";

		file_write(good_path, {0x12, 0x34, 0x56, 0x78});
		file_write(bad_path, {0x12, 0x34, 0x56});

		FooReader reader;

		Foo foo = reader.read_file(good_path);
		EXPECT_EQ(0x1234, foo.bar);
		EXPECT_EQ(0x7856, foo.baz);

		EXPECT_THROW(reader.read_file(bad_path), FooReader::Exception);
		EXPECT_THROW(reader.read_file(does_not_exist_path), FooReader::Exception);
	}

	TEST(BinaryTypeReader, ReadData)
	{
		FooReader reader;

		Foo foo = reader.read_data({0x12, 0x34, 0x56, 0x78});
		EXPECT_EQ(0x1234, foo.bar);
		EXPECT_EQ(0x7856, foo.baz);

		EXPECT_THROW(reader.read_data({0x12, 0x34, 0x56}), FooReader::Exception);
	}
}
