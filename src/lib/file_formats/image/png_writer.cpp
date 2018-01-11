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

#include <png.h>

#include <cstring>
#include <string>

#include "lib/file_formats/image/png_writer.h"
#include "lib/image.h"

namespace Rayni
{
	void PNGWriter::write_file(const std::string &file_name, const Image &image)
	{
		png_image pngimage;

		std::memset(&pngimage, 0, sizeof pngimage);
		pngimage.version = PNG_IMAGE_VERSION;
		pngimage.width = image.width();
		pngimage.height = image.height();
		pngimage.format = PNG_FORMAT_BGRA;

		if (!png_image_write_to_file(&pngimage,
		                             file_name.c_str(),
		                             0,
		                             image.buffer().data(),
		                             static_cast<png_int_32>(image.stride()),
		                             nullptr))
			throw Exception(file_name, pngimage.message);
	}
}
