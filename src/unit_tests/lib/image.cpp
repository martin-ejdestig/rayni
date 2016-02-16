/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2016 Martin Ejdestig <marejde@gmail.com>
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

#include <cmath>
#include <vector>

#include "lib/color.h"
#include "lib/image.h"

namespace Rayni
{
	class ImageTest : public testing::Test
	{
	protected:
		struct Pixel
		{
			unsigned int x, y;
			Color color;
		};

		static const std::vector<Pixel> &pixels_2x2()
		{
			static const std::vector<Pixel> pixels = {{0, 0, Color::black()},
			                                          {0, 1, Color::red()},
			                                          {1, 0, Color::green()},
			                                          {1, 1, Color::blue()}};
			return pixels;
		}

		static void write_pixels(Image &image, const std::vector<Pixel> &pixels)
		{
			for (auto &pixel : pixels)
				image.write_pixel(pixel.x, pixel.y, pixel.color);
		}

		static testing::AssertionResult expect_pixels(const char *image_expr,
		                                              const char *pixels_expr,
		                                              const Image &image,
		                                              const std::vector<Pixel> &pixels)
		{
			auto color_near = [](auto c1, auto c2)
			{
				static const real_t COMPONENT_MAX_DIFF = 0.001;

				return std::abs(c1.r() - c2.r()) <= COMPONENT_MAX_DIFF &&
				       std::abs(c1.g() - c2.g()) <= COMPONENT_MAX_DIFF &&
				       std::abs(c1.b() - c2.b()) <= COMPONENT_MAX_DIFF;
			};

			for (auto &pixel : pixels)
			{
				if (pixel.x > image.get_width() || pixel.y > image.get_height() ||
				    !color_near(pixel.color, image.read_pixel(pixel.x, pixel.y)))
					return testing::AssertionFailure() << image_expr << " does not contain "
					                                   << pixels_expr << ".";
			}

			return testing::AssertionSuccess();
		}
	};

	TEST_F(ImageTest, Size)
	{
		const unsigned int WIDTH = 4;
		const unsigned int HEIGHT = 2;
		Image image(WIDTH, HEIGHT);
		Image empty_image;

		EXPECT_FALSE(image.is_empty());
		EXPECT_EQ(WIDTH, image.get_width());
		EXPECT_EQ(HEIGHT, image.get_height());
		EXPECT_LE(WIDTH, image.get_stride());

		EXPECT_TRUE(empty_image.is_empty());
		EXPECT_EQ(0, empty_image.get_width());
		EXPECT_EQ(0, empty_image.get_height());
		EXPECT_EQ(0, empty_image.get_stride());
	}

	TEST_F(ImageTest, Area)
	{
		const unsigned int WIDTH = 4;
		const unsigned int HEIGHT = 2;
		Image::Area area = Image(WIDTH, HEIGHT).whole_area();

		EXPECT_EQ(0, area.x);
		EXPECT_EQ(0, area.y);
		EXPECT_EQ(WIDTH, area.width);
		EXPECT_EQ(HEIGHT, area.height);
	}

	TEST_F(ImageTest, Pixels)
	{
		Image image(2, 2);
		write_pixels(image, pixels_2x2());
		EXPECT_PRED_FORMAT2(expect_pixels, image, pixels_2x2());
	}

	TEST_F(ImageTest, MoveConstructor)
	{
		Image image1(2, 2);
		write_pixels(image1, pixels_2x2());
		Image image2(std::move(image1));

		EXPECT_PRED_FORMAT2(expect_pixels, image2, pixels_2x2());
		EXPECT_TRUE(image1.is_empty());
	}

	TEST_F(ImageTest, MoveAssignment)
	{
		Image image1(2, 2);
		write_pixels(image1, pixels_2x2());
		Image image2 = std::move(image1);

		EXPECT_PRED_FORMAT2(expect_pixels, image2, pixels_2x2());
		EXPECT_TRUE(image1.is_empty());
	}
}
