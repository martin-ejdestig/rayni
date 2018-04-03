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

#include "lib/file_formats/image/tga_reader.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lib/file_formats/write_to_file.h"
#include "lib/image.h"
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

	TEST(TGAReader, ReadFile)
	{
		static constexpr unsigned int VALID_WIDTH = 2;
		static constexpr unsigned int VALID_HEIGHT = 2;
		static constexpr Color VALID_COLORS[VALID_HEIGHT][VALID_WIDTH] = {{Color::red(), Color::yellow()},
		                                                                  {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;

		const std::string valid_path = temp_dir.path() / "valid.tga";
		write_to_file(valid_path, tga_data());
		Image image = TGAReader().read_file(valid_path);

		ASSERT_EQ(VALID_WIDTH, image.width());
		ASSERT_EQ(VALID_HEIGHT, image.height());

		for (unsigned int y = 0; y < VALID_HEIGHT; y++)
		{
			for (unsigned int x = 0; x < VALID_WIDTH; x++)
			{
				Color valid_color = VALID_COLORS[y][x];
				Color color = image.read_pixel(x, y);

				EXPECT_NEAR(valid_color.r(), color.r(), 1e-100);
				EXPECT_NEAR(valid_color.g(), color.g(), 1e-100);
				EXPECT_NEAR(valid_color.b(), color.b(), 1e-100);
			}
		}

		const std::string corrupt_path = temp_dir.path() / "corrupt.tga";
		write_to_file(corrupt_path, corrupt_tga_data());
		EXPECT_THROW(TGAReader().read_file(corrupt_path), TGAReader::Exception);

		const std::string short_path = temp_dir.path() / "short.tga";
		write_to_file(short_path, short_tga_data());
		EXPECT_THROW(TGAReader().read_file(short_path), TGAReader::Exception);

		EXPECT_THROW(TGAReader().read_file(temp_dir.path() / "does_not_exist.tga"), TGAReader::Exception);
	}

#if 0
	// TODO: Test more.
	TEST(TGAReader, HeaderShort)
	{
	}

	TEST(TGAReader, HeaderInvalidColorMap)
	{
	}

	TEST(TGAReader, HeaderInvalidImageType)
	{
	}

	TEST(TGAReader, HeaderMissingColorMap)
	{
	}

	TEST(TGAReader, HeaderColorMapPresentWhenItShouldNotBe)
	{
	}

	TEST(TGAReader, HeaderInvalidDimension)
	{
	}

	TEST(TGAReader, HeaderInvalidPixelSize)
	{
	}

	TEST(TGAReader, ImageData)
	{
	}

	TEST(TGAReader, ImageDataRLE)
	{
	}
#endif
}
