/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#include <cstdint>
#include <string>
#include <vector>

#include "lib/file_formats/image/png_reader.h"
#include "lib/file_formats/write_to_file.h"
#include "lib/image.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		std::vector<std::uint8_t> png_data()
		{
			return {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48,
			        0x44, 0x52, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x08, 0x02, 0x00, 0x00,
			        0x00, 0xfd, 0xd4, 0x9a, 0x73, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00,
			        0x00, 0x0b, 0x13, 0x00, 0x00, 0x0b, 0x13, 0x01, 0x00, 0x9a, 0x9c, 0x18, 0x00, 0x00,
			        0x00, 0x07, 0x74, 0x49, 0x4d, 0x45, 0x07, 0xe0, 0x07, 0x0f, 0x11, 0x10, 0x36, 0xa8,
			        0x4f, 0x33, 0xeb, 0x00, 0x00, 0x00, 0x1d, 0x69, 0x54, 0x58, 0x74, 0x43, 0x6f, 0x6d,
			        0x6d, 0x65, 0x6e, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x72, 0x65, 0x61, 0x74,
			        0x65, 0x64, 0x20, 0x77, 0x69, 0x74, 0x68, 0x20, 0x47, 0x49, 0x4d, 0x50, 0x64, 0x2e,
			        0x65, 0x07, 0x00, 0x00, 0x00, 0x0e, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xfc,
			        0xcf, 0xc0, 0xc0, 0x00, 0xc7, 0x00, 0x1c, 0x00, 0x03, 0xfe, 0x98, 0x28, 0x19, 0x3a,
			        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
		}

		std::vector<std::uint8_t> corrupt_png_data()
		{
			auto data = png_data();
			data.at(data.size() - 17) ^= 0x01;
			return data;
		}

		std::vector<std::uint8_t> short_png_data()
		{
			auto data = png_data();
			data.resize(data.size() - 13);
			return data;
		}
	}

	TEST(PNGReader, ReadFile)
	{
		static constexpr unsigned int VALID_WIDTH = 2;
		static constexpr unsigned int VALID_HEIGHT = 2;
		static constexpr Color VALID_COLORS[VALID_HEIGHT][VALID_WIDTH] = {{Color::red(), Color::yellow()},
		                                                                  {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;

		const std::string valid_path = temp_dir.path() / "valid.png";
		write_to_file(valid_path, png_data());
		Image image = PNGReader().read_file(valid_path);

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

		const std::string corrupt_path = temp_dir.path() / "corrupt.png";
		write_to_file(corrupt_path, corrupt_png_data());
		EXPECT_THROW(PNGReader().read_file(corrupt_path), PNGReader::Exception);

		const std::string short_path = temp_dir.path() / "short.png";
		write_to_file(short_path, short_png_data());
		EXPECT_THROW(PNGReader().read_file(short_path), PNGReader::Exception);

		EXPECT_THROW(PNGReader().read_file(temp_dir.path() / "does_not_exist.png"), PNGReader::Exception);
	}
}
