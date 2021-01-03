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

#include "lib/io/text_reader.h"

#include <cstring>
#include <string>
#include <utility>

namespace Rayni
{
	void TextReader::open_file(const std::string &file_name)
	{
		close();

		if (auto result = mmap_file_.map(file_name); !result)
			throw Exception(result.error().message());

		buffer_ = static_cast<const char *>(mmap_file_.data());
		buffer_size_ = mmap_file_.size();
		buffer_position_ = 0;

		position_ = Position(file_name);
		position_.next_line();
	}

	void TextReader::set_string(std::string &&string, const std::string &position_prefix)
	{
		close();

		string_ = std::move(string);

		buffer_ = string_.data();
		buffer_size_ = string_.length();
		buffer_position_ = 0;

		position_ = Position(position_prefix);
		position_.next_line();
	}

	void TextReader::close()
	{
		if (mmap_file_.data())
			mmap_file_.unmap();
		else
			string_ = std::string();

		buffer_ = nullptr;
		buffer_size_ = 0;
		buffer_position_ = 0;

		position_ = Position();
	}

	bool TextReader::skip_string(std::string_view str)
	{
		if (str.length() > buffer_size_ - buffer_position_)
			return false;

		if (std::memcmp(buffer_ + buffer_position_, str.data(), str.length()) != 0)
			return false;

		buffer_position_ += str.length();
		position_.next_columns(str.length());

		return true;
	}

	std::string TextReader::Position::to_string() const
	{
		if (line_ == 0)
			return prefix_;

		std::string str;

		if (!prefix_.empty())
			str += prefix_ + ":";

		str += std::to_string(line()) + ":" + std::to_string(column());

		return str;
	}
}
