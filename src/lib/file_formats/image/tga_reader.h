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

#ifndef RAYNI_LIB_FILE_FORMATS_IMAGE_TGA_READER_H
#define RAYNI_LIB_FILE_FORMATS_IMAGE_TGA_READER_H

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

#include "lib/file_formats/file_format_exception.h"
#include "lib/image.h"

namespace Rayni
{
	class TGAReader
	{
	public:
		using Exception = FileFormatException;

		Image read_file(const std::string &file_name);

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

		Image read_stream(std::istream &stream, const std::string &error_prefix);

		void read_header();
		void read_color_map();
		Image read_image_data();

		void read(void *dest, std::size_t size);

		template <typename T>
		void read(T &dest)
		{
			read(dest.data(), dest.size());
		}

		void read_run_length_encoded(std::vector<std::uint8_t> &dest);

		void skip(std::size_t size);

		unsigned int bytes_per_pixel() const;
		unsigned int x_to_image_x(unsigned int x) const;
		unsigned int y_to_image_y(unsigned int y) const;
		Color pixel_to_color(const std::vector<std::uint8_t> &bytes, unsigned int pixel_offset) const;

		ColorMapType byte_to_color_map_type(std::uint8_t byte) const;
		ImageType byte_to_image_type(std::uint8_t byte) const;

		std::string error_prefix;
		std::istream *stream = nullptr;

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
		} header;

		struct
		{
			bool raw = false;
			unsigned int bytes_left = 0;
			unsigned int pixel_pos = 0;
			std::uint8_t pixel[4] = {0, 0, 0, 0};
		} rle_state;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_IMAGE_TGA_READER_H
