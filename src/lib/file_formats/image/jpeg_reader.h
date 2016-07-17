/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2016 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FILE_FORMATS_IMAGE_JPEG_READER_H
#define RAYNI_LIB_FILE_FORMATS_IMAGE_JPEG_READER_H

#include <cstdio>
#include <string>

#include <jpeglib.h>

#include "lib/file_formats/file_format_exception.h"
#include "lib/image.h"

namespace Rayni
{
	class JPEGReader
	{
	public:
		using Exception = FileFormatException;

		Image read_file(const std::string &file_name);

	private:
		struct Callbacks;
		struct ErrorManager;

		bool decode_file_to_image(std::FILE &file, Image &image) const;
		bool color_space_requires_manual_conversion(J_COLOR_SPACE color_space) const;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_IMAGE_JPEG_READER_H
