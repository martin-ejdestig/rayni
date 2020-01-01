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

#include "lib/file_formats/object/ply_reader.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "lib/shapes/triangle_mesh_data.h"

// TODO: Test much more.
// - Test that corrupt header and data is handled gracefully (Hit every error
//   condition in ply_reader.cpp at least once, leave out element and property
//   names, negative element count etc.).
// - Make sure that many more different types (int*, float* and list) and format
//   (ASCII and binary) combinations are handled.
// - Verify that unknown elements and properties are skipped when reading data.
// - Check that triangulation of faces with more than 3 vertices work.
// - Predicate function(s?) for comparing mesh data instead of checking each
//   Vector3 element and its components manually.
// - Etc. etc...

namespace Rayni
{
	namespace
	{
		using PLYData = std::vector<std::uint8_t>;

		void append(PLYData &data, std::string string_to_append)
		{
			for (auto &c : string_to_append)
				data.push_back(std::uint8_t(c));
		}

		void append(PLYData &data, std::initializer_list<std::uint8_t> data_to_append)
		{
			for (auto &byte : data_to_append)
				data.push_back(byte);
		}

		PLYData basic_header(const std::string &format, unsigned int vertex_count, unsigned int face_count)
		{
			PLYData header;

			append(header, "ply\n");
			append(header, "format " + format + " 1.0\n");
			append(header, "element vertex " + std::to_string(vertex_count) + "\n");
			append(header, "property float x\n");
			append(header, "property float y\n");
			append(header, "property float z\n");
			append(header, "element face " + std::to_string(face_count) + "\n");
			append(header, "property list uint8 uint8 vertex_indices\n");
			append(header, "end_header\n");

			return header;
		}

		TriangleMeshData read(PLYData &&data)
		{
			return PLYReader().read_data(std::move(data));
		}
	}

	TEST(PLYReader, BasicASCII)
	{
		PLYData data = basic_header("ascii", 3, 1);
		append(data, "1 2 3\n");
		append(data, "4 5 6\n");
		append(data, "7 8 9\n");
		append(data, "3 0 1 2\n");

		auto mesh_data = read(std::move(data));

		ASSERT_EQ(3, mesh_data.points.size());
		EXPECT_NEAR(1, mesh_data.points[0].x(), 1e-7);
		EXPECT_NEAR(2, mesh_data.points[0].y(), 1e-7);
		EXPECT_NEAR(3, mesh_data.points[0].z(), 1e-7);
		EXPECT_NEAR(4, mesh_data.points[1].x(), 1e-7);
		EXPECT_NEAR(5, mesh_data.points[1].y(), 1e-7);
		EXPECT_NEAR(6, mesh_data.points[1].z(), 1e-7);
		EXPECT_NEAR(7, mesh_data.points[2].x(), 1e-7);
		EXPECT_NEAR(8, mesh_data.points[2].y(), 1e-7);
		EXPECT_NEAR(9, mesh_data.points[2].z(), 1e-7);

		ASSERT_EQ(1, mesh_data.indices.size());
		EXPECT_EQ(0, mesh_data.indices[0].index1);
		EXPECT_EQ(1, mesh_data.indices[0].index2);
		EXPECT_EQ(2, mesh_data.indices[0].index3);
	}

	TEST(PLYReader, BasicBinaryBigEndian)
	{
		PLYData data = basic_header("binary_big_endian", 3, 1);
		append(data, {0x3f, 0x80, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00});
		append(data, {0x40, 0x80, 0x00, 0x00, 0x40, 0xa0, 0x00, 0x00, 0x40, 0xc0, 0x00, 0x00});
		append(data, {0x40, 0xe0, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x41, 0x10, 0x00, 0x00});
		append(data, {0x03, 0x00, 0x01, 0x02});

		auto mesh_data = read(std::move(data));

		ASSERT_EQ(3, mesh_data.points.size());
		EXPECT_NEAR(1, mesh_data.points[0].x(), 1e-7);
		EXPECT_NEAR(2, mesh_data.points[0].y(), 1e-7);
		EXPECT_NEAR(3, mesh_data.points[0].z(), 1e-7);
		EXPECT_NEAR(4, mesh_data.points[1].x(), 1e-7);
		EXPECT_NEAR(5, mesh_data.points[1].y(), 1e-7);
		EXPECT_NEAR(6, mesh_data.points[1].z(), 1e-7);
		EXPECT_NEAR(7, mesh_data.points[2].x(), 1e-7);
		EXPECT_NEAR(8, mesh_data.points[2].y(), 1e-7);
		EXPECT_NEAR(9, mesh_data.points[2].z(), 1e-7);

		ASSERT_EQ(1, mesh_data.indices.size());
		EXPECT_EQ(0, mesh_data.indices[0].index1);
		EXPECT_EQ(1, mesh_data.indices[0].index2);
		EXPECT_EQ(2, mesh_data.indices[0].index3);
	}

	TEST(PLYReader, BasicBinaryLittleEndian)
	{
		PLYData data = basic_header("binary_little_endian", 3, 1);
		append(data, {0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x40});
		append(data, {0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0xc0, 0x40});
		append(data, {0x00, 0x00, 0xe0, 0x40, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x10, 0x41});
		append(data, {0x03, 0x00, 0x01, 0x02});

		auto mesh_data = read(std::move(data));

		ASSERT_EQ(3, mesh_data.points.size());
		EXPECT_NEAR(1, mesh_data.points[0].x(), 1e-7);
		EXPECT_NEAR(2, mesh_data.points[0].y(), 1e-7);
		EXPECT_NEAR(3, mesh_data.points[0].z(), 1e-7);
		EXPECT_NEAR(4, mesh_data.points[1].x(), 1e-7);
		EXPECT_NEAR(5, mesh_data.points[1].y(), 1e-7);
		EXPECT_NEAR(6, mesh_data.points[1].z(), 1e-7);
		EXPECT_NEAR(7, mesh_data.points[2].x(), 1e-7);
		EXPECT_NEAR(8, mesh_data.points[2].y(), 1e-7);
		EXPECT_NEAR(9, mesh_data.points[2].z(), 1e-7);

		ASSERT_EQ(1, mesh_data.indices.size());
		EXPECT_EQ(0, mesh_data.indices[0].index1);
		EXPECT_EQ(1, mesh_data.indices[0].index2);
		EXPECT_EQ(2, mesh_data.indices[0].index3);
	}

	TEST(PLYReader, MagicMismatch)
	{
		PLYData data = basic_header("ascii", 3, 1);
		data[0] = 'x'; // Corrupt first byte in magic.
		append(data, "0 0 0\n");
		append(data, "0 0 0\n");
		append(data, "0 0 0\n");
		append(data, "3 0 1 2\n");

		EXPECT_THROW(read(std::move(data)), PLYReader::Exception);
	}

	TEST(PLYReader, UnknownFormat)
	{
		PLYData data = basic_header("ascii_garbage", 3, 1);
		append(data, "0 0 0\n");
		append(data, "0 0 0\n");
		append(data, "0 0 0\n");
		append(data, "3 0 1 2\n");

		EXPECT_THROW(read(std::move(data)), PLYReader::Exception);
	}
}
