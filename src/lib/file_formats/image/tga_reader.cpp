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

#include "lib/file_formats/image/tga_reader.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <ios>
#include <string>

#include "lib/color.h"
#include "lib/image.h"
#include "lib/math/enum.h"

namespace Rayni
{
	Image TGAReader::read_file(const std::string &file_name)
	{
		std::ifstream file(file_name, std::ios::binary);
		if (!file.is_open())
			throw Exception(file_name, "failed to open TGA image");

		return read_stream(file, file_name);
	}

	Image TGAReader::read_stream(std::istream &stream, const std::string &error_prefix)
	{
		this->stream = &stream;
		this->error_prefix = error_prefix;

		read_header();

		skip(header.id_field_length);

		if (header.color_map_type == ColorMapType::PRESENT)
			read_color_map();

		return read_image_data();
	}

	void TGAReader::read_header()
	{
		std::array<std::uint8_t, 18> data;
		read(data);

		header.id_field_length = data[0];
		header.color_map_type = byte_to_color_map_type(data[1]);
		header.image_type = byte_to_image_type(data[2] & 0x07);
		header.run_length_encoded = (data[2] & 0x08) != 0;

		header.color_map.origin = data[3] + data[4] * 256;
		header.color_map.length = data[5] + data[6] * 256;
		header.color_map.entry_size = data[7];

		header.image.x_origin = data[8] + data[9] * 256;
		header.image.y_origin = data[10] + data[11] * 256;
		header.image.width = data[12] + data[13] * 256;
		header.image.height = data[14] + data[15] * 256;
		header.image.pixel_size = data[16];
		header.image.descriptor = data[17];

		if (header.image_type == ImageType::COLOR_MAPPED)
		{
			if (header.color_map.length == 0 || header.color_map.entry_size == 0 ||
			    header.color_map_type == ColorMapType::ABSCENT)
				throw Exception(error_prefix, "missing color map in color mapped TGA image");
		}
		else
		{
			if (header.color_map.length != 0 || header.color_map.entry_size != 0 ||
			    header.color_map_type == ColorMapType::PRESENT)
				throw Exception(error_prefix, "color map found in RGB/Mono TGA image");
		}

		if (header.image.width == 0 || header.image.height == 0)
			throw Exception(error_prefix, "invalid image dimensions in TGA image");

		if (header.image.pixel_size != 8 && header.image.pixel_size != 15 && header.image.pixel_size != 16 &&
		    header.image.pixel_size != 24 && header.image.pixel_size != 32)
			throw Exception(error_prefix, "invalid pixel depth in TGA image");
	}

	void TGAReader::read_color_map()
	{
		throw Exception(error_prefix, "support for color mapped TGA images not implemented");
	}

	Image TGAReader::read_image_data()
	{
		Image image(header.image.width, header.image.height);
		std::vector<std::uint8_t> row(bytes_per_pixel() * header.image.width);

		for (unsigned int y = 0; y < header.image.height; y++)
		{
			if (header.run_length_encoded)
				read_run_length_encoded(row);
			else
				read(row);

			for (unsigned int x = 0; x < header.image.width; x++)
				image.write_pixel(x_to_image_x(x), y_to_image_y(y), pixel_to_color(row, x));
		}

		return image;
	}

	void TGAReader::read(void *dest, std::size_t size)
	{
		stream->read(static_cast<std::istream::char_type *>(dest), static_cast<std::streamsize>(size));

		if (!stream->good())
		{
			if (stream->eof())
				throw Exception(error_prefix, "unexpected eof when reading TGA image");
			else
				throw Exception(error_prefix, "failed to read from TGA image");
		}
	}

	void TGAReader::read_run_length_encoded(std::vector<std::uint8_t> &dest)
	{
		std::size_t pos = 0;

		while (pos < dest.size())
		{
			if (rle_state.bytes_left == 0)
			{
				std::uint8_t repetition_count;
				read(&repetition_count, 1);

				rle_state.raw = repetition_count < 0x80;

				if (rle_state.raw)
				{
					rle_state.bytes_left = (repetition_count + 1u) * bytes_per_pixel();
				}
				else
				{
					rle_state.bytes_left = (repetition_count - 127u) * bytes_per_pixel();
					rle_state.pixel_pos = 0;
					read(rle_state.pixel, bytes_per_pixel());
				}
			}

			if (rle_state.raw)
			{
				auto size = std::min(dest.size() - pos, static_cast<std::size_t>(rle_state.bytes_left));
				read(&dest[pos], size);
				rle_state.bytes_left -= size;
				pos += size;
			}
			else
			{
				while (pos < dest.size() && rle_state.bytes_left > 0)
				{
					dest[pos++] = rle_state.pixel[rle_state.pixel_pos++];
					rle_state.bytes_left--;
					if (rle_state.pixel_pos >= bytes_per_pixel())
						rle_state.pixel_pos = 0;
				}
			}
		}
	}

	void TGAReader::skip(std::size_t size)
	{
		if (size == 0)
			return;

		stream->seekg(static_cast<std::istream::off_type>(size), std::ios::cur);

		if (!stream->good())
			throw Exception(error_prefix, "failed to seek in TGA image");
	}

	unsigned int TGAReader::bytes_per_pixel() const
	{
		return (header.image.pixel_size + 7u) / 8;
	}

	unsigned int TGAReader::x_to_image_x(unsigned int x) const
	{
		bool right_to_left = (header.image.descriptor & 0x10) != 0;
		return right_to_left ? header.image.width - 1u - x : x;
	}

	unsigned int TGAReader::y_to_image_y(unsigned int y) const
	{
		bool top_to_bottom = (header.image.descriptor & 0x20) != 0;
		return top_to_bottom ? y : header.image.height - 1u - y;
	}

	Color TGAReader::pixel_to_color(const std::vector<std::uint8_t> &bytes, unsigned int pixel_offset) const
	{
		unsigned int byte_offset = pixel_offset * bytes_per_pixel();
		assert(byte_offset < bytes.size());
		const std::uint8_t *pixel = &bytes[byte_offset];

		if (header.image_type == ImageType::RGB)
		{
			if (header.image.pixel_size == 24)
				return Color(real_t(pixel[2]) / 255, real_t(pixel[1]) / 255, real_t(pixel[0]) / 255);

			if (header.image.pixel_size == 32)
				return Color(real_t(pixel[2] * pixel[3]) / (255 * 255),
				             real_t(pixel[1] * pixel[3]) / (255 * 255),
				             real_t(pixel[0] * pixel[3]) / (255 * 255));
		}
		else if (header.image_type == ImageType::MONO)
		{
			if (header.image.pixel_size == 8)
				return Color(real_t(pixel[0]) / 255, real_t(pixel[0]) / 255, real_t(pixel[0]) / 255);
		}

		throw Exception(error_prefix, "unsupported TGA image type");
	}

	TGAReader::ColorMapType TGAReader::byte_to_color_map_type(std::uint8_t byte) const
	{
		auto i = enum_from_value({ColorMapType::ABSCENT, ColorMapType::PRESENT}, byte);

		if (!i)
			throw Exception(error_prefix, "unknown color map type field in TGA header");

		return i.value();
	}

	TGAReader::ImageType TGAReader::byte_to_image_type(std::uint8_t byte) const
	{
		auto i = enum_from_value({ImageType::NONE, ImageType::COLOR_MAPPED, ImageType::RGB, ImageType::MONO},
		                         byte);
		if (!i)
			throw Exception(error_prefix, "unknown image type field in TGA header");

		return i.value();
	}
}
