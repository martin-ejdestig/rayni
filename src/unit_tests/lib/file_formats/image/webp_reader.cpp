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

#include "lib/file_formats/image/webp_reader.h"

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
		std::vector<std::uint8_t> webp_data()
		{
			return {0x52, 0x49, 0x46, 0x46, 0x2a, 0x00, 0x00, 0x00, 0x57, 0x45, 0x42, 0x50, 0x56,
			        0x50, 0x38, 0x4c, 0x1e, 0x00, 0x00, 0x00, 0x2f, 0x01, 0x40, 0x00, 0x00, 0x1f,
			        0x20, 0x10, 0x48, 0xda, 0x1f, 0x7a, 0x8d, 0xf9, 0x17, 0x10, 0x14, 0xf9, 0x3f,
			        0xda, 0xfc, 0x07, 0x5f, 0x24, 0x60, 0x17, 0x22, 0xfa, 0x1f, 0x01};
		}

		std::vector<std::uint8_t> corrupt_webp_data()
		{
			auto data = webp_data();
			data.at(data.size() - 6) ^= 0x01;
			return data;
		}

		std::vector<std::uint8_t> short_webp_data()
		{
			auto data = webp_data();
			data.pop_back();
			return data;
		}
	}

	TEST(WebPReader, ReadFile)
	{
		static constexpr unsigned int VALID_WIDTH = 2;
		static constexpr unsigned int VALID_HEIGHT = 2;
		static constexpr Color VALID_COLORS[VALID_HEIGHT][VALID_WIDTH] = {{Color::red(), Color::yellow()},
		                                                                  {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;

		const std::string valid_path = temp_dir.path() / "valid.webp";
		ASSERT_TRUE(file_write(valid_path, webp_data()));
		Image image = WebPReader().read_file(valid_path);

		ASSERT_EQ(VALID_WIDTH, image.width());
		ASSERT_EQ(VALID_HEIGHT, image.height());

		for (unsigned int y = 0; y < VALID_HEIGHT; y++)
		{
			for (unsigned int x = 0; x < VALID_WIDTH; x++)
			{
				Color color = image.read_pixel(x, y);
				EXPECT_NEAR(VALID_COLORS[y][x].r(), color.r(), 1e-100);
				EXPECT_NEAR(VALID_COLORS[y][x].g(), color.g(), 1e-100);
				EXPECT_NEAR(VALID_COLORS[y][x].b(), color.b(), 1e-100);
			}
		}

		const std::string corrupt_path = temp_dir.path() / "corrupt.webp";
		ASSERT_TRUE(file_write(corrupt_path, corrupt_webp_data()));
		EXPECT_THROW(WebPReader().read_file(corrupt_path), WebPReader::Exception);

		const std::string short_path = temp_dir.path() / "short.webp";
		ASSERT_TRUE(file_write(short_path, short_webp_data()));
		EXPECT_THROW(WebPReader().read_file(short_path), WebPReader::Exception);

		EXPECT_THROW(WebPReader().read_file(temp_dir.path() / "does_not_exist.webp"), WebPReader::Exception);
	}
}
