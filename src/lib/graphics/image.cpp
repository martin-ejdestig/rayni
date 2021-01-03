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

#include "lib/graphics/image.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "lib/file_formats/image.h"
#include "lib/graphics/color.h"

namespace Rayni
{
	Image::Image() = default;

	Image::Image(Image &&other) noexcept :
	        width_(std::exchange(other.width_, 0)),
	        height_(std::exchange(other.height_, 0)),
	        buffer_(std::move(other.buffer_))
	{
	}

	Image::Image(unsigned int width, unsigned int height) :
	        width_(width),
	        height_(height),
	        buffer_(stride() * height)
	{
	}

	Result<Image> Image::from_variant(const Variant &v)
	{
		auto path = v.get<std::string>("path");
		if (!path)
			return path.error();

		return image_read_file(*path);
	}

	Image &Image::operator=(Image &&other) noexcept
	{
		assert(this != &other);

		width_ = std::exchange(other.width_, 0);
		height_ = std::exchange(other.height_, 0);
		buffer_ = std::move(other.buffer_);

		return *this;
	}

	void Image::write_pixel(unsigned int x, unsigned int y, const Color &color)
	{
		std::size_t i = offset_to(x, y);

		buffer_[i + R_PIXEL_OFFSET] = color.r() < 1 ? static_cast<std::uint8_t>(color.r() * 256) : 255;
		buffer_[i + G_PIXEL_OFFSET] = color.g() < 1 ? static_cast<std::uint8_t>(color.g() * 256) : 255;
		buffer_[i + B_PIXEL_OFFSET] = color.b() < 1 ? static_cast<std::uint8_t>(color.b() * 256) : 255;
	}

	Color Image::read_pixel(unsigned int x, unsigned int y) const
	{
		Color color;
		std::size_t i = offset_to(x, y);

		color.r() = static_cast<real_t>(buffer_[i + R_PIXEL_OFFSET]) / 255;
		color.g() = static_cast<real_t>(buffer_[i + G_PIXEL_OFFSET]) / 255;
		color.b() = static_cast<real_t>(buffer_[i + B_PIXEL_OFFSET]) / 255;

		return color;
	}
}
