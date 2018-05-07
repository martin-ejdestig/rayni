/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_GRAPHICS_IMAGE_H
#define RAYNI_LIB_GRAPHICS_IMAGE_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "lib/containers/variant.h"
#include "lib/graphics/color.h"
#include "lib/math/math.h"

namespace Rayni
{
	class Image
	{
	public:
		struct Area;

		Image();
		Image(const Image &other) = delete;
		Image(Image &&other) noexcept;
		Image(unsigned int width, unsigned int height);

		~Image() = default;

		static Image from_variant(const Variant &v);

		Image &operator=(const Image &other) = delete;
		Image &operator=(Image &&other) noexcept;

		bool is_empty() const
		{
			return width() == 0 || height() == 0;
		}

		unsigned int width() const
		{
			return width_;
		}

		unsigned int height() const
		{
			return height_;
		}

		unsigned int stride() const
		{
			return width() * BYTES_PER_PIXEL;
		}

		std::vector<std::uint8_t> &buffer()
		{
			return buffer_;
		}

		const std::vector<std::uint8_t> &buffer() const
		{
			return buffer_;
		}

		std::uint8_t &start_of_row(unsigned int y)
		{
			return buffer()[offset_to(0, y)];
		}

		Area whole_area() const;

		void write_pixel(unsigned int x, unsigned int y, const Color &color);
		Color read_pixel(unsigned int x, unsigned int y) const;

	private:
		// Order in memory is BBGGRRAA. I.e. ARGB32 stored in little-endian order. Alpha is
		// always set to 0xff. The reason for including an alpha channel is to avoid pixel
		// conversions when drawing an Image to the screen. Some toolkits (e.g. Cairo) do
		// not support RGB24.
		static constexpr unsigned int A_PIXEL_OFFSET = 3;
		static constexpr unsigned int R_PIXEL_OFFSET = 2;
		static constexpr unsigned int G_PIXEL_OFFSET = 1;
		static constexpr unsigned int B_PIXEL_OFFSET = 0;
		static constexpr unsigned int BYTES_PER_PIXEL = 4;

		unsigned int offset_to(unsigned int x, unsigned int y) const
		{
			assert(x < width() && y < height());
			return stride() * y + x * BYTES_PER_PIXEL;
		}

		unsigned int width_ = 0;
		unsigned int height_ = 0;
		std::vector<std::uint8_t> buffer_;
	};

	struct Image::Area
	{
		Area() = default;

		Area(unsigned int x, unsigned int y, unsigned int width, unsigned int height) :
		        x(x),
		        y(y),
		        width(width),
		        height(height)
		{
		}

		unsigned int x = 0;
		unsigned int y = 0;
		unsigned int width = 0;
		unsigned int height = 0;
	};

	inline Image::Area Image::whole_area() const
	{
		return {0, 0, width(), height()};
	}
}

#endif // RAYNI_LIB_GRAPHICS_IMAGE_H
