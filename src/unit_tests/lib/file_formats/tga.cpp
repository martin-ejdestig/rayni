// This file is part of Rayni.
//
// Copyright (C) 2016-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/tga.h"

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
		std::vector<std::uint8_t> tga_data()
		{
			return {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x18, 0x00, 0x00, 0xff,
			        0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0xff};
		}

		std::vector<std::uint8_t> corrupt_tga_data()
		{
			auto data = tga_data();
			data.at(16) ^= 0x01;
			return data;
		}

		std::vector<std::uint8_t> short_tga_data()
		{
			auto data = tga_data();
			data.pop_back();
			return data;
		}
	}

	TEST(TGAReadFile, Valid)
	{
		static constexpr unsigned int VALID_WIDTH = 2;
		static constexpr unsigned int VALID_HEIGHT = 2;
		static constexpr Color VALID_COLORS[VALID_HEIGHT][VALID_WIDTH] = {{Color::red(), Color::yellow()},
		                                                                  {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "valid.tga";
		ASSERT_TRUE(file_write(path, tga_data()));
		Image image = tga_read_file(path).value_or(Image());

		ASSERT_EQ(VALID_WIDTH, image.width());
		ASSERT_EQ(VALID_HEIGHT, image.height());

		for (unsigned int y = 0; y < VALID_HEIGHT; y++) {
			for (unsigned int x = 0; x < VALID_WIDTH; x++) {
				Color valid_color = VALID_COLORS[y][x];
				Color color = image.read_pixel(x, y);

				EXPECT_NEAR(valid_color.r(), color.r(), 1e-100);
				EXPECT_NEAR(valid_color.g(), color.g(), 1e-100);
				EXPECT_NEAR(valid_color.b(), color.b(), 1e-100);
			}
		}
	}

	TEST(TGAReadFile, Corrupt)
	{
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "corrupt.tga";
		ASSERT_TRUE(file_write(path, corrupt_tga_data()));

		EXPECT_FALSE(tga_read_file(path));
	}

	TEST(TGAReadFile, Short)
	{
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "short.tga";
		ASSERT_TRUE(file_write(path, short_tga_data()));

		EXPECT_FALSE(tga_read_file(path));
	}

	TEST(TGAReadFile, DoesNotExist)
	{
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "does_not_exist.tga";

		EXPECT_FALSE(tga_read_file(path));
	}

#if 0
	// TODO: Test more.
	TEST(TGAReadFile, HeaderShort)
	{
	}

	TEST(TGAReadFile, HeaderInvalidColorMap)
	{
	}

	TEST(TGAReadFile, HeaderInvalidImageType)
	{
	}

	TEST(TGAReadFile, HeaderMissingColorMap)
	{
	}

	TEST(TGAReadFile, HeaderColorMapPresentWhenItShouldNotBe)
	{
	}

	TEST(TGAReadFile, HeaderInvalidDimension)
	{
	}

	TEST(TGAReadFile, HeaderInvalidPixelSize)
	{
	}

	TEST(TGAReadFile, ImageData)
	{
	}

	TEST(TGAReadFile, ImageDataRLE)
	{
	}
#endif
}
