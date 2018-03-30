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

#ifndef RAYNI_LIB_FILE_FORMATS_BINARY_TYPE_READER_H
#define RAYNI_LIB_FILE_FORMATS_BINARY_TYPE_READER_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "lib/file_formats/binary_reader.h"
#include "lib/function/scope_exit.h"

namespace Rayni
{
	// BinaryTypeReader is a convenience class for creating a BinaryReader with methods that
	// return a specific type as a result of reading a file or data.
	template <typename T>
	class BinaryTypeReader : protected BinaryReader
	{
	public:
		using Exception = BinaryReader::Exception;

		virtual ~BinaryTypeReader() = default;

		T read_file(const std::string &file_name)
		{
			auto closer = scope_exit([&] { close(); });
			open_file(file_name);
			return read();
		}

		T read_data(std::vector<std::uint8_t> &&data)
		{
			return read_data(std::move(data), "");
		}

		T read_data(std::vector<std::uint8_t> &&data, const std::string &position_prefix)
		{
			auto closer = scope_exit([&] { close(); });
			set_data(std::move(data), position_prefix);
			return read();
		}

	private:
		virtual T read() = 0;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_BINARY_TYPE_READER_H
