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

#include "lib/file_formats/tga.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "lib/function/result.h"
#include "lib/graphics/color.h"
#include "lib/graphics/image.h"
#include "lib/io/binary_reader.h"

namespace Rayni
{
	namespace
	{
		using Exception = BinaryReader::Exception;

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

		struct Header
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
				std::uint8_t bytes_per_pixel = 0; // For convenience. Not in actual header.
				std::uint8_t descriptor = 0;
			} image;
		};

		struct RLEState
		{
			bool raw = false;
			unsigned int bytes_left = 0;
			unsigned int pixel_pos = 0;
			std::array<std::uint8_t, 4> pixel = {0, 0, 0, 0};
		};

		std::optional<ColorMapType> byte_to_color_map_type(std::uint8_t byte)
		{
			if (byte == static_cast<std::uint8_t>(ColorMapType::ABSCENT))
				return ColorMapType::ABSCENT;
			if (byte == static_cast<std::uint8_t>(ColorMapType::PRESENT))
				return ColorMapType::PRESENT;

			return {};
		}

		std::optional<ImageType> byte_to_image_type(std::uint8_t byte)
		{
			if (byte == static_cast<std::uint8_t>(ImageType::NONE))
				return ImageType::NONE;
			if (byte == static_cast<std::uint8_t>(ImageType::COLOR_MAPPED))
				return ImageType::COLOR_MAPPED;
			if (byte == static_cast<std::uint8_t>(ImageType::RGB))
				return ImageType::RGB;
			if (byte == static_cast<std::uint8_t>(ImageType::MONO))
				return ImageType::MONO;

			return {};
		}

		Header read_header(BinaryReader &reader)
		{
			Header header;
			std::array<std::uint8_t, 18> data;

			reader.read_bytes(data);

			header.id_field_length = data[0];

			auto color_map_type = byte_to_color_map_type(data[1]);
			if (!color_map_type)
				throw Exception(reader.position(), "unknown color map type field in TGA header");
			header.color_map_type = *color_map_type;

			auto image_type = byte_to_image_type(data[2] & 0x07);
			if (!image_type)
				throw Exception(reader.position(), "unknown image type field in TGA header");
			header.image_type = *image_type;

			header.run_length_encoded = (data[2] & 0x08) != 0;

			header.color_map.origin = data[3] + data[4] * 256;
			header.color_map.length = data[5] + data[6] * 256;
			header.color_map.entry_size = data[7];

			header.image.x_origin = data[8] + data[9] * 256;
			header.image.y_origin = data[10] + data[11] * 256;
			header.image.width = data[12] + data[13] * 256;
			header.image.height = data[14] + data[15] * 256;
			header.image.pixel_size = data[16];
			header.image.bytes_per_pixel = (header.image.pixel_size + 7U) / 8; // For convenience.
			header.image.descriptor = data[17];

			if (header.image_type == ImageType::COLOR_MAPPED)
			{
				if (header.color_map.length == 0 || header.color_map.entry_size == 0 ||
				    header.color_map_type == ColorMapType::ABSCENT)
					throw Exception(reader.position(),
					                "missing color map in color mapped TGA image");
			}
			else
			{
				if (header.color_map.length != 0 || header.color_map.entry_size != 0 ||
				    header.color_map_type == ColorMapType::PRESENT)
					throw Exception(reader.position(), "color map found in RGB/Mono TGA image");
			}

			if (header.image.width == 0 || header.image.height == 0)
				throw Exception(reader.position(), "invalid image dimensions in TGA image");

			if (header.image.pixel_size != 8 && header.image.pixel_size != 15 &&
			    header.image.pixel_size != 16 && header.image.pixel_size != 24 &&
			    header.image.pixel_size != 32)
				throw Exception(reader.position(), "invalid pixel depth in TGA image");

			return header;
		}

		void read_run_length_encoded(BinaryReader &reader,
		                             const Header &header,
		                             RLEState &rle_state,
		                             std::vector<std::uint8_t> &dest)
		{
			std::size_t pos = 0;

			while (pos < dest.size())
			{
				if (rle_state.bytes_left == 0)
				{
					std::uint8_t repetition_count = reader.read_uint8();

					rle_state.raw = repetition_count < 0x80;

					if (rle_state.raw)
					{
						rle_state.bytes_left =
						        (repetition_count + 1U) * header.image.bytes_per_pixel;
					}
					else
					{
						rle_state.bytes_left =
						        (repetition_count - 127U) * header.image.bytes_per_pixel;
						rle_state.pixel_pos = 0;
						reader.read_bytes(rle_state.pixel, header.image.bytes_per_pixel);
					}
				}

				if (rle_state.raw)
				{
					auto size = std::min(dest.size() - pos,
					                     static_cast<std::size_t>(rle_state.bytes_left));
					reader.read_bytes(dest, pos, size);
					rle_state.bytes_left -= size;
					pos += size;
				}
				else
				{
					while (pos < dest.size() && rle_state.bytes_left > 0)
					{
						dest[pos++] = rle_state.pixel[rle_state.pixel_pos++];
						rle_state.bytes_left--;
						if (rle_state.pixel_pos >= header.image.bytes_per_pixel)
							rle_state.pixel_pos = 0;
					}
				}
			}
		}

		Image read_image_data(BinaryReader &reader, const Header &header)
		{
			Image image(header.image.width, header.image.height);
			std::vector<std::uint8_t> row(unsigned(header.image.bytes_per_pixel * header.image.width));
			RLEState rle_state;
			bool right_to_left = (header.image.descriptor & 0x10) != 0;
			bool top_to_bottom = (header.image.descriptor & 0x20) != 0;

			for (unsigned int y = 0; y < header.image.height; y++)
			{
				if (header.run_length_encoded)
					read_run_length_encoded(reader, header, rle_state, row);
				else
					reader.read_bytes(row);

				for (unsigned int x = 0; x < header.image.width; x++)
				{
					unsigned int image_x = right_to_left ? header.image.width - 1U - x : x;
					unsigned int image_y = top_to_bottom ? y : header.image.height - 1U - y;
					const std::uint8_t *pixel = &row[header.image.bytes_per_pixel * x];
					Color color;

					if (header.image_type == ImageType::RGB && header.image.pixel_size == 24)
					{
						color = {real_t(pixel[2]) / 255,
						         real_t(pixel[1]) / 255,
						         real_t(pixel[0]) / 255};
					}
					else if (header.image_type == ImageType::RGB && header.image.pixel_size == 32)
					{
						color = {real_t(pixel[2] * pixel[3]) / (255 * 255),
						         real_t(pixel[1] * pixel[3]) / (255 * 255),
						         real_t(pixel[0] * pixel[3]) / (255 * 255)};
					}
					else if (header.image_type == ImageType::MONO && header.image.pixel_size == 8)
					{
						color = {real_t(pixel[0]) / 255,
						         real_t(pixel[0]) / 255,
						         real_t(pixel[0]) / 255};
					}
					else
					{
						throw Exception(reader.position(), "unsupported TGA image type");
					}

					image.write_pixel(image_x, image_y, color);
				}
			}

			return image;
		}

		Image read_tga(BinaryReader &reader)
		{
			Header header = read_header(reader);

			reader.skip_bytes(header.id_field_length);

			if (header.color_map_type == ColorMapType::PRESENT)
				throw Exception(reader.position(),
				                "support for color mapped TGA images not implemented");

			return read_image_data(reader, header);
		}
	}

	Result<Image> tga_read_file(const std::string &file_name)
	{
		Image image;

		try
		{
			BinaryReader reader;
			reader.open_file(file_name);
			image = read_tga(reader);
		}
		catch (const BinaryReader::Exception &e)
		{
			return Error(e.what());
		}

		return image;
	}
}
