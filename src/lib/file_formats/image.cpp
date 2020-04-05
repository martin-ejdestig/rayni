// This file is part of Rayni.
//
// Copyright (C) 2014-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/image.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <string>

#include "lib/file_formats/exr_reader.h"
#include "lib/file_formats/jpeg_reader.h"
#include "lib/file_formats/png_reader.h"
#include "lib/file_formats/tga_reader.h"
#include "lib/file_formats/webp_reader.h"
#include "lib/string/string.h"
#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	namespace
	{
		ImageFormat image_format_from_magic(const std::string &file_name)
		{
			MemoryMappedFile file;
			if (!file.map(file_name))
				return ImageFormat::UNDETERMINED;

			auto magic_matches = [&](std::uint64_t offset,
			                         const std::initializer_list<std::uint8_t> &magic) {
				if (offset + magic.size() > file.size())
					return false;

				auto p = static_cast<const std::uint8_t *>(file.data()) + offset;

				for (auto &byte : magic)
					if (byte != *p++)
						return false;

				return true;
			};

			if (magic_matches(0, {0x76, 0x2f, 0x31, 0x01}))
				return ImageFormat::EXR;

			if (magic_matches(0, {0xff, 0xd8, 0xff}))
				return ImageFormat::JPEG;

			if (magic_matches(0, {0x89, 'P', 'N', 'G'}))
				return ImageFormat::PNG;

			if (magic_matches(0, {'R', 'I', 'F', 'F'}) && magic_matches(8, {'W', 'E', 'B', 'P'}))
				return ImageFormat::WEBP;

			return ImageFormat::UNDETERMINED;
		}

		ImageFormat image_format_from_extension(const std::string &file_name)
		{
			const std::string extension = string_to_lower(std::filesystem::path(file_name).extension());

			auto extension_is_any_of = [&](const std::initializer_list<const char *> &strs) {
				return std::find(strs.begin(), strs.end(), extension) != strs.end();
			};

			if (extension_is_any_of({".exr"}))
				return ImageFormat::EXR;

			if (extension_is_any_of({".jpg", ".jpeg", ".jpe"}))
				return ImageFormat::JPEG;

			if (extension_is_any_of({".png"}))
				return ImageFormat::PNG;

			if (extension_is_any_of({".icb", ".targa", ".tga", ".tpic", ".vda", ".vst"}))
				return ImageFormat::TGA;

			if (extension_is_any_of({".webp"}))
				return ImageFormat::WEBP;

			return ImageFormat::UNDETERMINED;
		}
	}

	// TODO: Result<Image> image_read_file(const std::string &file_name) => no longer a member function.
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	Image ImageReader::read_file(const std::string &file_name)
	{
		ImageFormat format = image_format_from_file(file_name);

		switch (format)
		{
		case ImageFormat::EXR:
			return EXRReader().read_file(file_name);

		case ImageFormat::JPEG:
			return JPEGReader().read_file(file_name);

		case ImageFormat::PNG:
			return PNGReader().read_file(file_name);

		case ImageFormat::TGA:
			return TGAReader().read_file(file_name);

		case ImageFormat::WEBP:
			return WebPReader().read_file(file_name);

		case ImageFormat::UNDETERMINED:
			break;
		}

		throw Exception(file_name, "unable to determine image format");
	}

	ImageFormat image_format_from_file(const std::string &file_name)
	{
		ImageFormat type = image_format_from_magic(file_name);

		if (type == ImageFormat::UNDETERMINED)
			type = image_format_from_extension(file_name);

		return type;
	}
}
