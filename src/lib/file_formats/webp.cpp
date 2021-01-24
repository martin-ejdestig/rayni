// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/webp.h"

#include <webp/decode.h>

#include <cstdint>

#include "lib/function/result.h"
#include "lib/graphics/image.h"
#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	Result<Image> webp_read_file(const std::string &file_name)
	{
		MemoryMappedFile file;
		if (auto r = file.map(file_name); !r)
			return r.error();

		int width;
		int height;
		if (!WebPGetInfo(static_cast<const std::uint8_t *>(file.data()), file.size(), &width, &height))
			return Error(file_name + ": failed to determine size of WebP image");

		Image image(static_cast<unsigned int>(width), static_cast<unsigned int>(height));

		if (!WebPDecodeRGBInto(static_cast<const std::uint8_t *>(file.data()),
		                       file.size(),
		                       image.buffer().data(),
		                       image.buffer().size(),
		                       static_cast<int>(image.stride())))
			return Error(file_name + ": failed to decode WebP image");

		return image;
	}
}
