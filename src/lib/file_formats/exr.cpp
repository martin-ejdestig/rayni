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

#include "lib/file_formats/exr.h"

#include <tinyexr.h>

#include "lib/function/scope_exit.h"
#include "lib/graphics/color.h"

namespace Rayni
{
	Result<Image> exr_read_file(const std::string &file_name)
	{
		float *out = nullptr;
		auto out_free = scope_exit([&] { std::free(out); });
		int width;
		int height;
		const char *err = nullptr;

		if (LoadEXR(&out, &width, &height, file_name.c_str(), &err) != TINYEXR_SUCCESS)
		{
			if (err)
				FreeEXRErrorMessage(err);
			return Error(file_name + ": failed to read EXR image");
		}

		if (width <= 0 || height <= 0)
			return Error(file_name + ": invalid size (" + std::to_string(width) + "," +
			             std::to_string(height) + ") in EXR image");

		Image image(static_cast<unsigned int>(width), static_cast<unsigned int>(height));

		for (unsigned int y = 0; y < image.height(); y++)
		{
			for (unsigned int x = 0; x < image.width(); x++)
			{
				const float *rgba = out + (y * image.width() + x) * 4;
				Color c;
				c.r() = rgba[0] * rgba[3];
				c.g() = rgba[1] * rgba[3];
				c.b() = rgba[2] * rgba[3];
				image.write_pixel(x, y, c);
			}
		}

		return image;
	}
}
