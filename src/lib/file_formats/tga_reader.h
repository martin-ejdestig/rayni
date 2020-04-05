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

#ifndef RAYNI_LIB_FILE_FORMATS_TGA_READER_H
#define RAYNI_LIB_FILE_FORMATS_TGA_READER_H

#include <array>
#include <cstdint>
#include <vector>

#include "lib/graphics/image.h"
#include "lib/io/binary_type_reader.h"

namespace Rayni
{
	class TGAReader : public BinaryTypeReader<Image>
	{
	private:
		enum class ColorMapType : std::uint8_t
		{
			ABSCENT = 0,
			PRESENT = 1
		};

		enum class ImageType : std::uint8_t
		{
			NONE = 0,
			COLOR_MAPPED = 1,
			RGB = 2,
			MONO = 3
		};

		Image read() override;

		void read_header();
		void read_color_map();
		Image read_image_data();

		void read_run_length_encoded(std::vector<std::uint8_t> &dest);

		unsigned int bytes_per_pixel() const;
		unsigned int x_to_image_x(unsigned int x) const;
		unsigned int y_to_image_y(unsigned int y) const;
		Color pixel_to_color(const std::vector<std::uint8_t> &bytes, unsigned int pixel_offset) const;

		ColorMapType byte_to_color_map_type(std::uint8_t byte) const;
		ImageType byte_to_image_type(std::uint8_t byte) const;

		struct
		{
			std::uint8_t id_field_length = 0;
			ColorMapType color_map_type = ColorMapType::ABSCENT;
			ImageType image_type = ImageType::NONE;
			bool run_length_encoded = false;

			struct
			{
				std::uint16_t origin = 0;
				std::uint16_t length = 0;
				std::uint8_t entry_size = 0;
			} color_map;

			struct
			{
				std::uint16_t x_origin = 0;
				std::uint16_t y_origin = 0;
				std::uint16_t width = 0;
				std::uint16_t height = 0;
				std::uint8_t pixel_size = 0;
				std::uint8_t descriptor = 0;
			} image;
		} header_;

		struct
		{
			bool raw = false;
			unsigned int bytes_left = 0;
			unsigned int pixel_pos = 0;
			std::array<std::uint8_t, 4> pixel = {0, 0, 0, 0};
		} rle_state_;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_TGA_READER_H
