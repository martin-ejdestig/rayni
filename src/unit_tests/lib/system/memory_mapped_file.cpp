// This file is part of Rayni.
//
// Copyright (C) 2017-2021 Martin Ejdestig <marejde@gmail.com>
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

#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	TEST(MemoryMappedFile, Map)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "file";
		const std::vector<std::uint8_t> bytes = {0x12, 0x98, 0x34, 0x76};
		ASSERT_TRUE(file_write(file_name, bytes));

		MemoryMappedFile file;
		ASSERT_TRUE(file.map(file_name));

		ASSERT_EQ(bytes.size(), file.size());
		EXPECT_EQ(0, std::memcmp(bytes.data(), file.data(), bytes.size()));
	}

	TEST(MemoryMappedFile, MapAlreadyMapped)
	{
		ScopedTempDir temp_dir;
		const std::string file_name1 = temp_dir.path() / "file1";
		const std::string file_name2 = temp_dir.path() / "file2";
		const std::vector<std::uint8_t> bytes1 = {0xa1, 0xE7, 0x6d, 0x83};
		const std::vector<std::uint8_t> bytes2 = {0xff, 0x6b};
		ASSERT_TRUE(file_write(file_name1, bytes1));
		ASSERT_TRUE(file_write(file_name2, bytes2));

		MemoryMappedFile file;

		ASSERT_TRUE(file.map(file_name1));
		ASSERT_EQ(bytes1.size(), file.size());
		EXPECT_EQ(0, std::memcmp(bytes1.data(), file.data(), bytes1.size()));

		ASSERT_TRUE(file.map(file_name2));
		ASSERT_EQ(2U, file.size());
		EXPECT_EQ(0, std::memcmp(bytes2.data(), file.data(), bytes2.size()));
	}

	TEST(MemoryMappedFile, MapFileThatDoesNotExist)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "does_not_exist";

		MemoryMappedFile file;
		EXPECT_FALSE(file.map(file_name));
	}

	TEST(MemoryMappedFile, MapZeroBytesFile)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "empty_file";
		const std::vector<std::uint8_t> bytes = {};
		ASSERT_TRUE(file_write(file_name, bytes));

		MemoryMappedFile file;
		ASSERT_TRUE(file.map(file_name));
		EXPECT_EQ(nullptr, file.data());
		EXPECT_EQ(0, file.size());
	}

	TEST(MemoryMappedFile, UnmapResetsDataAndSize)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "file";
		const std::vector<std::uint8_t> bytes = {0x12, 0x98, 0x34, 0x76};
		ASSERT_TRUE(file_write(file_name, bytes));

		MemoryMappedFile file;
		EXPECT_EQ(nullptr, file.data());
		EXPECT_EQ(0, file.size());

		ASSERT_TRUE(file.map(file_name));
		EXPECT_NE(nullptr, file.data());
		EXPECT_NE(0, file.size());

		file.unmap();
		EXPECT_EQ(nullptr, file.data());
		EXPECT_EQ(0, file.size());
	}

	TEST(MemoryMappedFile, UnmapIfNotMappedDoesNothing)
	{
		MemoryMappedFile file;
		EXPECT_EQ(nullptr, file.data());
		EXPECT_EQ(0, file.size());

		file.unmap();
		EXPECT_EQ(nullptr, file.data());
		EXPECT_EQ(0, file.size());
	}

	TEST(MemoryMappedFile, MoveConstructor)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "file";
		const std::vector<std::uint8_t> bytes = {0x12};
		ASSERT_TRUE(file_write(file_name, bytes));

		MemoryMappedFile file1;
		ASSERT_TRUE(file1.map(file_name));

		MemoryMappedFile file2(std::move(file1));

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ(nullptr, file1.data());
		EXPECT_EQ(0, file1.size());
		EXPECT_NE(nullptr, file2.data());
		EXPECT_NE(0, file2.size());
	}

	TEST(MemoryMappedFile, MoveAssignment)
	{
		ScopedTempDir temp_dir;
		const std::string file_name = temp_dir.path() / "file";
		const std::vector<std::uint8_t> bytes = {0x12};
		ASSERT_TRUE(file_write(file_name, bytes));

		MemoryMappedFile file1;
		ASSERT_TRUE(file1.map(file_name));

		MemoryMappedFile file2;
		file2 = std::move(file1);

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_EQ(nullptr, file1.data());
		EXPECT_EQ(0, file1.size());
		EXPECT_NE(nullptr, file2.data());
		EXPECT_NE(0, file2.size());
	}
}
