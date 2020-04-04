// This file is part of Rayni.
//
// Copyright (C) 2013-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/graphics/image.h"

#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include "lib/graphics/color.h"
#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		struct Pixel
		{
			unsigned int x, y;
			Color color;
		};

		testing::AssertionResult expect_color_at(const char *image_expr,
		                                         const char * /*x_expr*/,
		                                         const char * /*y_expr*/,
		                                         const char *color_expr,
		                                         const Image &image,
		                                         unsigned int x,
		                                         unsigned int y,
		                                         const Color &color)
		{
			static constexpr real_t COMPONENT_MAX_DIFF = 0.001;

			if (x >= image.width() || y >= image.height())
				return testing::AssertionFailure()
				       << image_expr << " does not contain (" << x << ", " << y << ") (too small)";

			Color diff = color - image.read_pixel(x, y);

			if (std::abs(diff.r()) > COMPONENT_MAX_DIFF || std::abs(diff.g()) > COMPONENT_MAX_DIFF ||
			    std::abs(diff.b()) > COMPONENT_MAX_DIFF)
			{
				return testing::AssertionFailure()
				       << color_expr << " and " << image_expr << " color at (" << x << ", " << y
				       << ") componentwise difference is (" << diff.r() << ", " << diff.g() << ", "
				       << diff.b() << ").";
			}

			return testing::AssertionSuccess();
		}

		const std::vector<Pixel> &pixels_2x2()
		{
			static const std::vector<Pixel> pixels = {{0, 0, Color::black()},
			                                          {0, 1, Color::red()},
			                                          {1, 0, Color::green()},
			                                          {1, 1, Color::blue()}};
			return pixels;
		}

		void write_pixels(Image &image, const std::vector<Pixel> &pixels)
		{
			for (auto &pixel : pixels)
				image.write_pixel(pixel.x, pixel.y, pixel.color);
		}

		testing::AssertionResult expect_pixels(const char *image_expr,
		                                       const char *pixels_expr,
		                                       const Image &image,
		                                       const std::vector<Pixel> &pixels)
		{
			for (std::vector<Pixel>::size_type i = 0; i < pixels.size(); i++)
			{
				std::string pixels_expr_i = std::string(pixels_expr) + "[" + std::to_string(i) + "]";
				auto result = expect_color_at(image_expr,
				                              (pixels_expr_i + ".x").c_str(),
				                              (pixels_expr_i + ".y").c_str(),
				                              (pixels_expr_i + ".color").c_str(),
				                              image,
				                              pixels[i].x,
				                              pixels[i].y,
				                              pixels[i].color);
				if (!result)
					return result;
			}

			return testing::AssertionSuccess();
		}
	}

	TEST(Image, Variant)
	{
		ScopedTempDir temp_dir;
		const std::string path = temp_dir.path() / "image.tga";
		const std::string missing_path = temp_dir.path() / "missing.tga";

		ASSERT_TRUE(file_write(path, {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		                              0x00, 0x01, 0x00, 0x01, 0x00, 0x18, 0x00, 0xff, 0xff, 0xff}));

		Image image = Variant::map("path", path).to<Image>();
		EXPECT_PRED_FORMAT4(expect_color_at, image, 0, 0, Color::white());

		EXPECT_THROW(Variant::map("path", missing_path).to<Image>(), Variant::Exception);

		EXPECT_THROW(Variant::map().to<Image>(), Variant::Exception);
	}

	TEST(Image, Size)
	{
		constexpr unsigned int WIDTH = 4;
		constexpr unsigned int HEIGHT = 2;
		Image image(WIDTH, HEIGHT);
		Image empty_image;

		EXPECT_FALSE(image.is_empty());
		EXPECT_EQ(WIDTH, image.width());
		EXPECT_EQ(HEIGHT, image.height());
		EXPECT_LE(WIDTH, image.stride());

		EXPECT_TRUE(empty_image.is_empty());
		EXPECT_EQ(0, empty_image.width());
		EXPECT_EQ(0, empty_image.height());
		EXPECT_EQ(0, empty_image.stride());
	}

	TEST(Image, Area)
	{
		constexpr unsigned int WIDTH = 4;
		constexpr unsigned int HEIGHT = 2;
		Image::Area area = Image(WIDTH, HEIGHT).whole_area();

		EXPECT_EQ(0, area.x);
		EXPECT_EQ(0, area.y);
		EXPECT_EQ(WIDTH, area.width);
		EXPECT_EQ(HEIGHT, area.height);
	}

	TEST(Image, BlackByDefault)
	{
		Image image(2, 2);

		for (unsigned int y = 0; y < image.height(); y++)
			for (unsigned int x = 0; x < image.width(); x++)
				EXPECT_PRED_FORMAT4(expect_color_at, image, x, y, Color::black());
	}

	TEST(Image, Pixels)
	{
		Image image(2, 2);
		write_pixels(image, pixels_2x2());
		EXPECT_PRED_FORMAT2(expect_pixels, image, pixels_2x2());
	}

	TEST(Image, MoveConstructor)
	{
		Image image1(2, 2);
		write_pixels(image1, pixels_2x2());
		Image image2(std::move(image1));

		EXPECT_PRED_FORMAT2(expect_pixels, image2, pixels_2x2());

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_TRUE(image1.is_empty());
	}

	TEST(Image, MoveAssignment)
	{
		Image image1(2, 2);
		write_pixels(image1, pixels_2x2());
		Image image2 = std::move(image1);

		EXPECT_PRED_FORMAT2(expect_pixels, image2, pixels_2x2());

		// NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move) Tests move.
		EXPECT_TRUE(image1.is_empty());
	}
}
