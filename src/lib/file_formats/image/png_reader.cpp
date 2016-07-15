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

#include "lib/file_formats/image/png_reader.h"

#include <png.h>

#include <cstring>
#include <string>
#include <vector>

#include "lib/image.h"

namespace Rayni
{
	struct PNGReader::PNGImageFreer
	{
		explicit PNGImageFreer(png_image &image) : image(image)
		{
		}

		~PNGImageFreer()
		{
			png_image_free(&image);
		}

		png_image &image;
	};

	Image PNGReader::read_file(const std::string &file_name)
	{
		png_image pngimage;
		std::memset(&pngimage, 0, sizeof pngimage);
		pngimage.version = PNG_IMAGE_VERSION;
		PNGImageFreer pngimage_freer(pngimage);

		if (!png_image_begin_read_from_file(&pngimage, file_name.c_str()))
			throw Exception(file_name, pngimage.message);

		pngimage.format = PNG_FORMAT_BGRA;
		Image image(pngimage.width, pngimage.height);

		if (!png_image_finish_read(&pngimage,
		                           nullptr,
		                           image.buffer().data(),
		                           static_cast<png_int_32>(image.stride()),
		                           nullptr))
			throw Exception(file_name, pngimage.message);

		return image;
	}
}
