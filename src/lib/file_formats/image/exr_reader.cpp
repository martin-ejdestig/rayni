// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/image/exr_reader.h"

#if defined __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-register"
#	pragma clang diagnostic ignored "-Wold-style-cast"
#	pragma clang diagnostic ignored "-Wsign-conversion"
#	pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wdeprecated"
#	pragma GCC diagnostic ignored "-Wdeprecated-copy"
#	pragma GCC diagnostic ignored "-Wold-style-cast"
#	pragma GCC diagnostic ignored "-Wshadow"
#	pragma GCC diagnostic ignored "-Wsign-conversion"
#	pragma GCC diagnostic ignored "-Wsuggest-override"
#	pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#include <IexBaseExc.h>
#include <ImathBox.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>

#if defined __clang__
#	pragma clang diagnostic pop
#elif defined __GNUC__
#	pragma GCC diagnostic pop
#endif

#include <cstddef>
#include <string>
#include <vector>

#include "lib/graphics/color.h"
#include "lib/graphics/image.h"

namespace Rayni
{
	// TODO: Result<Image> exr_read_file(const std::string &file_name) => no longer a member function.
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	Image EXRReader::read_file(const std::string &file_name)
	{
		Image image;
		std::vector<Imf::Rgba> pixels;

		try
		{
			Imf::RgbaInputFile file(file_name.c_str());
			Imath::Box2i data_window = file.dataWindow();
			int width = data_window.max.x - data_window.min.x + 1;
			int height = data_window.max.y - data_window.min.y + 1;

			if (width <= 0 || height <= 0)
				throw Exception(file_name,
				                "invalid size (" + std::to_string(width) + "," +
				                        std::to_string(height) + ") in EXR image");

			image = Image(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
			pixels.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));

			file.setFrameBuffer(&pixels[0] - data_window.min.x - data_window.min.y * width,
			                    1,
			                    static_cast<std::size_t>(width));
			file.readPixels(data_window.min.y, data_window.max.y);
		}
		catch (const Iex::BaseExc &e)
		{
			throw Exception(file_name, std::string("failed to read EXR image (") + e.what() + ")");
		}

		for (unsigned int y = 0; y < image.height(); y++)
		{
			for (unsigned int x = 0; x < image.width(); x++)
			{
				const Imf::Rgba &pixel = pixels[std::size_t(y) * image.width() + std::size_t(x)];
				image.write_pixel(x, y, Color(pixel.r * pixel.a, pixel.g * pixel.a, pixel.b * pixel.a));
			}
		}

		return image;
	}
}
