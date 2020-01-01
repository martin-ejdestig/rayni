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

#include "lib/file_formats/image/webp_reader.h"

#include <webp/decode.h>

#include <cstdint>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <vector>

#include "lib/graphics/image.h"

namespace Rayni
{
	// TODO: Result<Image> wepb_read_file(const std::string &file_name) => no longer a member function.
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	Image WebPReader::read_file(const std::string &file_name)
	{
		std::ifstream file(file_name, std::ios::binary);
		if (!file.is_open())
			throw Exception(file_name, "failed to open WebP image");

		std::vector<std::uint8_t> file_data((std::istreambuf_iterator<char>(file)),
		                                    std::istreambuf_iterator<char>());

		int width;
		int height;
		if (!WebPGetInfo(file_data.data(), file_data.size(), &width, &height))
			throw Exception(file_name, "failed to determine size of WebP image");

		Image image(static_cast<unsigned int>(width), static_cast<unsigned int>(height));

		if (!WebPDecodeRGBInto(file_data.data(),
		                       file_data.size(),
		                       image.buffer().data(),
		                       image.buffer().size(),
		                       static_cast<int>(image.stride())))
			throw Exception(file_name, "failed to decode WebP image");

		return image;
	}
}
