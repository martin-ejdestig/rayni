// This file is part of Rayni.
//
// Copyright (C) 2013-2019 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_IO_TEXT_TYPE_READER_H
#define RAYNI_LIB_IO_TEXT_TYPE_READER_H

#include <string>

#include "lib/function/scope_exit.h"
#include "lib/io/text_reader.h"

namespace Rayni
{
	// TextTypeReader is a convenience class for creating a TextReader with methods that
	// return a specific type as a result of parsing a file or a string.
	template <typename T>
	class TextTypeReader : protected TextReader
	{
	public:
		using Exception = TextReader::Exception;

		virtual ~TextTypeReader() = default;

		T read_file(const std::string &file_name)
		{
			auto closer = scope_exit([&] { close(); });
			open_file(file_name);
			return read();
		}

		T read_string(const std::string &string)
		{
			return read_string(string, "");
		}

		T read_string(const std::string &string, const std::string &position_prefix)
		{
			auto closer = scope_exit([&] { close(); });
			set_string(string, position_prefix);
			return read();
		}

	private:
		virtual T read() = 0;
	};
}

#endif // RAYNI_LIB_IO_TEXT_TYPE_READER_H
