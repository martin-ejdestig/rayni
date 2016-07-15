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

#include "lib/file_formats/image/image_format.h"

#include <algorithm>
#include <cstdint>
#include <experimental/filesystem>
#include <fstream>
#include <istream>
#include <string>
#include <vector>

#include "lib/string/string.h"

namespace Rayni
{
	ImageFormat::Type ImageFormat::determine_type_from_file(const std::string &file_name)
	{
		Type type = determine_type_from_magic(file_name);

		if (type == Type::UNDETERMINED)
			type = determine_type_from_extension(file_name);

		return type;
	}

	ImageFormat::Type ImageFormat::determine_type_from_magic(const std::string &file_name)
	{
		std::ifstream stream(file_name, std::ios::binary);
		if (!stream.is_open())
			return Type::UNDETERMINED;

		auto magic_matches = [&](std::istream::pos_type offset, const std::vector<std::uint8_t> magic) {
			stream.seekg(offset);

			for (auto &byte : magic)
				if (stream.get() != byte)
					return false;

			return true;
		};

		if (magic_matches(0, {0x76, 0x2f, 0x31, 0x01}))
			return Type::EXR;

		if (magic_matches(0, {0xff, 0xd8, 0xff}))
			return Type::JPEG;

		if (magic_matches(0, {0x89, 'P', 'N', 'G'}))
			return Type::PNG;

		if (magic_matches(0, {'R', 'I', 'F', 'F'}) && magic_matches(8, {'W', 'E', 'B', 'P'}))
			return Type::WEBP;

		return Type::UNDETERMINED;
	}

	ImageFormat::Type ImageFormat::determine_type_from_extension(const std::string &file_name)
	{
		const std::string extension =
		        string_to_lower(std::experimental::filesystem::path(file_name).extension());

		auto extension_is_any_of = [&](const std::vector<std::string> &strs) {
			return std::find(strs.begin(), strs.end(), extension) != strs.end();
		};

		if (extension_is_any_of({".exr"}))
			return Type::EXR;

		if (extension_is_any_of({".jpg", ".jpeg", ".jpe"}))
			return Type::JPEG;

		if (extension_is_any_of({".png"}))
			return Type::PNG;

		if (extension_is_any_of({".icb", ".targa", ".tga", ".tpic", ".vda", ".vst"}))
			return Type::TGA;

		if (extension_is_any_of({".webp"}))
			return Type::WEBP;

		return Type::UNDETERMINED;
	}
}
