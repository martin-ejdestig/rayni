/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2014-2018 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/image/image_reader.h"

#include <string>

#include "lib/file_formats/image/exr_reader.h"
#include "lib/file_formats/image/image_format.h"
#include "lib/file_formats/image/jpeg_reader.h"
#include "lib/file_formats/image/png_reader.h"
#include "lib/file_formats/image/tga_reader.h"
#include "lib/file_formats/image/webp_reader.h"

namespace Rayni
{
	Image ImageReader::read_file(const std::string &file_name)
	{
		ImageFormat format = image_format_from_file(file_name);

		switch (format)
		{
		case ImageFormat::EXR:
			return EXRReader().read_file(file_name);

		case ImageFormat::JPEG:
			return JPEGReader().read_file(file_name);

		case ImageFormat::PNG:
			return PNGReader().read_file(file_name);

		case ImageFormat::TGA:
			return TGAReader().read_file(file_name);

		case ImageFormat::WEBP:
			return WebPReader().read_file(file_name);

		case ImageFormat::UNDETERMINED:
			break;
		}

		throw Exception(file_name, "unable to determine image format");
	}
}
