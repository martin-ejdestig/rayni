/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2017 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/color.h"
#include "lib/file_formats/image/png_reader.h"
#include "lib/file_formats/image/png_writer.h"
#include "lib/image.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	TEST(PNGWriter, WriteFile)
	{
		static constexpr unsigned int WIDTH = 2;
		static constexpr unsigned int HEIGHT = 2;
		static constexpr Color COLORS[HEIGHT][WIDTH] = {{Color::red(), Color::yellow()},
		                                                {Color::green(), Color::blue()}};
		ScopedTempDir temp_dir;

		Image image(WIDTH, HEIGHT);
		for (unsigned int y = 0; y < HEIGHT; y++)
			for (unsigned int x = 0; x < WIDTH; x++)
				image.write_pixel(x, y, COLORS[y][x]);

		PNGWriter().write_file(temp_dir.path() / "foo.png", image);
		Image read_image = PNGReader().read_file(temp_dir.path() / "foo.png");

		ASSERT_EQ(WIDTH, read_image.width());
		ASSERT_EQ(HEIGHT, read_image.height());

		for (unsigned int y = 0; y < HEIGHT; y++)
		{
			for (unsigned int x = 0; x < WIDTH; x++)
			{
				Color expected_color = COLORS[y][x];
				Color color = read_image.read_pixel(x, y);

				EXPECT_NEAR(expected_color.r(), color.r(), 1e-100);
				EXPECT_NEAR(expected_color.g(), color.g(), 1e-100);
				EXPECT_NEAR(expected_color.b(), color.b(), 1e-100);
			}
		}

		EXPECT_THROW(PNGWriter().write_file(temp_dir.path() / "dir_that_does_not_exist" / "bar.png", image),
		             PNGWriter::Exception);
	}
}
