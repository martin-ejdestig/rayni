// This file is part of Rayni.
//
// Copyright (C) 2013-2021 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_GRAPHICS_IMAGE_H
#define RAYNI_LIB_GRAPHICS_IMAGE_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "lib/containers/variant.h"
#include "lib/function/result.h"
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

		static Result<Image> from_variant(const Variant &v);

		Image &operator=(const Image &other) = delete;
		Image &operator=(Image &&other) noexcept;

		bool is_empty() const
		{
			return width_ == 0 || height_ == 0;
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
			return width_ * BYTES_PER_PIXEL;
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
			return buffer_[offset_to(0, y)];
		}

		Area whole_area() const;

		void write_pixel(unsigned int x, unsigned int y, const Color &color);
		Color read_pixel(unsigned int x, unsigned int y) const;

	private:
		// Order in memory is RRGGBB. I.e. RGB24 stored in big-endian order.
		static constexpr unsigned int R_PIXEL_OFFSET = 0;
		static constexpr unsigned int G_PIXEL_OFFSET = 1;
		static constexpr unsigned int B_PIXEL_OFFSET = 2;
		static constexpr unsigned int BYTES_PER_PIXEL = 3;

		unsigned int offset_to(unsigned int x, unsigned int y) const
		{
			assert(x < width_ && y < height_);
			return stride() * y + x * BYTES_PER_PIXEL;
		}

		unsigned int width_ = 0;
		unsigned int height_ = 0;
		std::vector<std::uint8_t> buffer_;
	};

	struct Image::Area
	{
		Area() = default;

		Area(unsigned int x_in, unsigned int y_in, unsigned int width_in, unsigned int height_in) :
		        x(x_in),
		        y(y_in),
		        width(width_in),
		        height(height_in)
		{
		}

		unsigned int x = 0;
		unsigned int y = 0;
		unsigned int width = 0;
		unsigned int height = 0;
	};

	inline Image::Area Image::whole_area() const
	{
		return {0, 0, width_, height_};
	}
}

#endif // RAYNI_LIB_GRAPHICS_IMAGE_H
