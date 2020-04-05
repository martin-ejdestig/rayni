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

#include "lib/file_formats/tga.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <vector>

#include "lib/function/result.h"
#include "lib/graphics/color.h"
#include "lib/graphics/image.h"
#include "lib/io/binary_type_reader.h"

namespace Rayni
{
	namespace
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

		Image TGAReader::read()
		{
			read_header();

			skip_bytes(header_.id_field_length);

			if (header_.color_map_type == ColorMapType::PRESENT)
				read_color_map();

			return read_image_data();
		}

		void TGAReader::read_header()
		{
			std::array<std::uint8_t, 18> data;
			read_bytes(data);

			header_.id_field_length = data[0];
			header_.color_map_type = byte_to_color_map_type(data[1]);
			header_.image_type = byte_to_image_type(data[2] & 0x07);
			header_.run_length_encoded = (data[2] & 0x08) != 0;

			header_.color_map.origin = data[3] + data[4] * 256;
			header_.color_map.length = data[5] + data[6] * 256;
			header_.color_map.entry_size = data[7];

			header_.image.x_origin = data[8] + data[9] * 256;
			header_.image.y_origin = data[10] + data[11] * 256;
			header_.image.width = data[12] + data[13] * 256;
			header_.image.height = data[14] + data[15] * 256;
			header_.image.pixel_size = data[16];
			header_.image.descriptor = data[17];

			if (header_.image_type == ImageType::COLOR_MAPPED)
			{
				if (header_.color_map.length == 0 || header_.color_map.entry_size == 0 ||
				    header_.color_map_type == ColorMapType::ABSCENT)
					throw Exception(position(), "missing color map in color mapped TGA image");
			}
			else
			{
				if (header_.color_map.length != 0 || header_.color_map.entry_size != 0 ||
				    header_.color_map_type == ColorMapType::PRESENT)
					throw Exception(position(), "color map found in RGB/Mono TGA image");
			}

			if (header_.image.width == 0 || header_.image.height == 0)
				throw Exception(position(), "invalid image dimensions in TGA image");

			if (header_.image.pixel_size != 8 && header_.image.pixel_size != 15 &&
			    header_.image.pixel_size != 16 && header_.image.pixel_size != 24 &&
			    header_.image.pixel_size != 32)
				throw Exception(position(), "invalid pixel depth in TGA image");
		}

		void TGAReader::read_color_map()
		{
			throw Exception(position(), "support for color mapped TGA images not implemented");
		}

		Image TGAReader::read_image_data()
		{
			Image image(header_.image.width, header_.image.height);
			std::vector<std::uint8_t> row(bytes_per_pixel() * header_.image.width);

			for (unsigned int y = 0; y < header_.image.height; y++)
			{
				if (header_.run_length_encoded)
					read_run_length_encoded(row);
				else
					read_bytes(row);

				for (unsigned int x = 0; x < header_.image.width; x++)
					image.write_pixel(x_to_image_x(x), y_to_image_y(y), pixel_to_color(row, x));
			}

			return image;
		}

		void TGAReader::read_run_length_encoded(std::vector<std::uint8_t> &dest)
		{
			std::size_t pos = 0;

			while (pos < dest.size())
			{
				if (rle_state_.bytes_left == 0)
				{
					std::uint8_t repetition_count = read_uint8();

					rle_state_.raw = repetition_count < 0x80;

					if (rle_state_.raw)
					{
						rle_state_.bytes_left = (repetition_count + 1U) * bytes_per_pixel();
					}
					else
					{
						rle_state_.bytes_left = (repetition_count - 127U) * bytes_per_pixel();
						rle_state_.pixel_pos = 0;
						read_bytes(rle_state_.pixel, bytes_per_pixel());
					}
				}

				if (rle_state_.raw)
				{
					auto size = std::min(dest.size() - pos,
					                     static_cast<std::size_t>(rle_state_.bytes_left));
					read_bytes(dest, pos, size);
					rle_state_.bytes_left -= size;
					pos += size;
				}
				else
				{
					while (pos < dest.size() && rle_state_.bytes_left > 0)
					{
						dest[pos++] = rle_state_.pixel[rle_state_.pixel_pos++];
						rle_state_.bytes_left--;
						if (rle_state_.pixel_pos >= bytes_per_pixel())
							rle_state_.pixel_pos = 0;
					}
				}
			}
		}

		unsigned int TGAReader::bytes_per_pixel() const
		{
			return (header_.image.pixel_size + 7U) / 8;
		}

		unsigned int TGAReader::x_to_image_x(unsigned int x) const
		{
			bool right_to_left = (header_.image.descriptor & 0x10) != 0;
			return right_to_left ? header_.image.width - 1U - x : x;
		}

		unsigned int TGAReader::y_to_image_y(unsigned int y) const
		{
			bool top_to_bottom = (header_.image.descriptor & 0x20) != 0;
			return top_to_bottom ? y : header_.image.height - 1U - y;
		}

		Color TGAReader::pixel_to_color(const std::vector<std::uint8_t> &bytes, unsigned int pixel_offset) const
		{
			unsigned int byte_offset = pixel_offset * bytes_per_pixel();
			assert(byte_offset < bytes.size());
			const std::uint8_t *pixel = &bytes[byte_offset];

			if (header_.image_type == ImageType::RGB)
			{
				if (header_.image.pixel_size == 24)
					return {real_t(pixel[2]) / 255, real_t(pixel[1]) / 255, real_t(pixel[0]) / 255};

				if (header_.image.pixel_size == 32)
					return {real_t(pixel[2] * pixel[3]) / (255 * 255),
					        real_t(pixel[1] * pixel[3]) / (255 * 255),
					        real_t(pixel[0] * pixel[3]) / (255 * 255)};
			}
			else if (header_.image_type == ImageType::MONO)
			{
				if (header_.image.pixel_size == 8)
					return {real_t(pixel[0]) / 255, real_t(pixel[0]) / 255, real_t(pixel[0]) / 255};
			}

			throw Exception(position(), "unsupported TGA image type");
		}

		TGAReader::ColorMapType TGAReader::byte_to_color_map_type(std::uint8_t byte) const
		{
			if (byte == static_cast<std::uint8_t>(ColorMapType::ABSCENT))
				return ColorMapType::ABSCENT;
			if (byte == static_cast<std::uint8_t>(ColorMapType::PRESENT))
				return ColorMapType::PRESENT;

			throw Exception(position(), "unknown color map type field in TGA header");
		}

		TGAReader::ImageType TGAReader::byte_to_image_type(std::uint8_t byte) const
		{
			if (byte == static_cast<std::uint8_t>(ImageType::NONE))
				return ImageType::NONE;
			if (byte == static_cast<std::uint8_t>(ImageType::COLOR_MAPPED))
				return ImageType::COLOR_MAPPED;
			if (byte == static_cast<std::uint8_t>(ImageType::RGB))
				return ImageType::RGB;
			if (byte == static_cast<std::uint8_t>(ImageType::MONO))
				return ImageType::MONO;

			throw Exception(position(), "unknown image type field in TGA header");
		}
	}

	Result<Image> tga_read_file(const std::string &file_name)
	{
		Image image;

		try
		{
			image = TGAReader().read_file(file_name);
		}
		catch (const TGAReader::Exception &e)
		{
			return Error(e.what());
		}

		return image;
	}
}
