// This file is part of Rayni.
//
// Copyright (C) 2016-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/jpeg_reader.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lib/graphics/image.h"
#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		std::vector<std::uint8_t> jpeg_data()
		{
			return {0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x02,
			        0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43, 0x00, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0xdb, 0x00, 0x43, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x01, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x00, 0x02, 0x00, 0x02, 0x03,
			        0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01, 0xff, 0xc4, 0x00, 0x14, 0x00,
			        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x09, 0xff, 0xc4, 0x00, 0x1a, 0x10, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07,
			        0x08, 0x03, 0x02, 0xff, 0xc4, 0x00, 0x15, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0a, 0xff, 0xc4,
			        0x00, 0x1b, 0x11, 0x00, 0x03, 0x00, 0x03, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x05, 0x06, 0x03, 0x07, 0x08, 0x09, 0x02, 0xff,
			        0xda, 0x00, 0x0c, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3f, 0x00, 0x00,
			        0xf7, 0x9e, 0xe2, 0xed, 0x5c, 0xfb, 0x72, 0xd9, 0xa0, 0xa0, 0xbb, 0x03, 0xa8, 0xe2,
			        0x21, 0x62, 0x35, 0x7d, 0x12, 0x42, 0x2e, 0x2e, 0x43, 0xa0, 0x35, 0x89, 0xa9, 0x39,
			        0x09, 0x39, 0xaa, 0xf7, 0x09, 0xa7, 0x25, 0xe5, 0xe7, 0x13, 0x56, 0x84, 0x9d, 0x04,
			        0xea, 0x04, 0xe1, 0x06, 0xa5, 0x22, 0x45, 0x21, 0x88, 0xb5, 0x52, 0xd1, 0x06, 0x04,
			        0x11, 0xbc, 0x05, 0xf0, 0xf2, 0xf2, 0xf8, 0xa4, 0xbe, 0x0e, 0xf3, 0x47, 0xce, 0x2d,
			        0x8f, 0xc3, 0x9c, 0x65, 0xb0, 0xf6, 0x1f, 0x9f, 0xfc, 0x4b, 0x79, 0x7f, 0x79, 0xca,
			        0x3c, 0xed, 0x67, 0x73, 0x73, 0x67, 0xca, 0x9a, 0x26, 0xa2, 0xc2, 0xce, 0xc2, 0xa3,
			        0x50, 0xc7, 0xbc, 0xa6, 0xac, 0xac, 0xa6, 0x79, 0x06, 0x73, 0xaa, 0x2a, 0x5a, 0x27,
			        0x47, 0x1a, 0xe1, 0xeb, 0xd7, 0x06, 0x98, 0xd1, 0xbb, 0x43, 0x0a, 0x60, 0xc0, 0xa2,
			        0x0b, 0x23, 0x36, 0x6f, 0xb2, 0x1e, 0xa9, 0xdb, 0x9b, 0x5a, 0xbb, 0x57, 0x6b, 0x6a,
			        0xba, 0xbd, 0x9d, 0xb0, 0xa9, 0xea, 0x69, 0xe0, 0x63, 0xa8, 0x69, 0x69, 0x68, 0x6d,
			        0x29, 0x1d, 0x3f, 0xa1, 0x7e, 0xea, 0x75, 0x73, 0x27, 0x2f, 0x1e, 0x39, 0x64, 0xc8,
			        0x96, 0x2d, 0x9c, 0x36, 0x62, 0x49, 0x27, 0xb3, 0x66, 0x79, 0x24, 0x1a, 0x79, 0xa4,
			        0x67, 0x28, 0xac, 0xf9, 0x73, 0xe5, 0xc9, 0x93, 0xeb, 0xff, 0xd9};
		}

		std::vector<std::uint8_t> corrupt_jpeg_data()
		{
			auto data = jpeg_data();
			data.at(20) ^= 0x01;
			return data;
		}

		std::vector<std::uint8_t> short_jpeg_data()
		{
			auto data = jpeg_data();
			data.resize(data.size() - 198);
			return data;
		}
	}

	TEST(JPEGReader, ReadFile)
	{
		static constexpr unsigned int VALID_WIDTH = 2;
		static constexpr unsigned int VALID_HEIGHT = 2;
		static constexpr Color VALID_COLORS[VALID_HEIGHT][VALID_WIDTH] = {{Color::red(), Color::yellow()},
		                                                                  {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;

		const std::string valid_path = temp_dir.path() / "valid.jpg";
		ASSERT_TRUE(file_write(valid_path, jpeg_data()));
		Image image = JPEGReader().read_file(valid_path);

		ASSERT_EQ(VALID_WIDTH, image.width());
		ASSERT_EQ(VALID_HEIGHT, image.height());

		for (unsigned int y = 0; y < VALID_HEIGHT; y++)
		{
			for (unsigned int x = 0; x < VALID_WIDTH; x++)
			{
				Color valid_color = VALID_COLORS[y][x];
				Color color = image.read_pixel(x, y);

				EXPECT_NEAR(valid_color.r(), color.r(), 8e-3);
				EXPECT_NEAR(valid_color.g(), color.g(), 8e-3);
				EXPECT_NEAR(valid_color.b(), color.b(), 8e-3);
			}
		}

		const std::string corrupt_path = temp_dir.path() / "corrupt.jpg";
		ASSERT_TRUE(file_write(corrupt_path, corrupt_jpeg_data()));
		EXPECT_THROW(JPEGReader().read_file(corrupt_path), JPEGReader::Exception);

		const std::string short_path = temp_dir.path() / "short.jpg";
		ASSERT_TRUE(file_write(short_path, short_jpeg_data()));
		EXPECT_THROW(JPEGReader().read_file(short_path), JPEGReader::Exception);

		EXPECT_THROW(JPEGReader().read_file(temp_dir.path() / "does_not_exist.jpg"), JPEGReader::Exception);
	}
}