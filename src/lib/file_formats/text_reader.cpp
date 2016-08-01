/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2016 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/text_reader.h"

#include <cassert>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <utility>

namespace Rayni
{
	void TextReader::open_file(const std::string &file_name)
	{
		auto file = std::make_unique<std::ifstream>(file_name);
		if (!file->is_open())
			throw Exception(file_name, "failed to open file");
		reset(std::move(file), file_name);
	}

	void TextReader::set_string(const std::string &string, const std::string &position_prefix)
	{
		reset(std::make_unique<std::istringstream>(string), position_prefix);
	}

	void TextReader::reset(std::unique_ptr<std::istream> &&istream, const std::string &position_prefix)
	{
		this->istream = std::move(istream);
		line = "";
		position_ = Position(position_prefix);
		getline();
	}

	void TextReader::close()
	{
		istream.reset();
		line = "";
		position_ = Position();
	}

	bool TextReader::skip_string(const std::string &str)
	{
		if (str.length() > line.length() - position_.line_index())
			return false;

		if (line.compare(position_.line_index(), str.length(), str) != 0)
			return false;

		position_.next_columns(str.length());

		return true;
	}

	void TextReader::getline()
	{
		if (!istream)
			throw Exception("no file or string stream to read from");

		if (!std::getline(*istream, line))
		{
			if (istream->eof())
				throw EOFException(position_, "end of stream");
			else
				throw Exception(position_, "read error");
		}

		// NOTE: std::getline() strips '\n'. There is no way to detect if '\n' was in
		//       stream or not when end is reached. Remove use of std::getline()?
		line += '\n';

		position_.next_line();
	}

	std::string TextReader::Position::to_string() const
	{
		if (!is_set())
			return prefix;

		std::string str;

		if (!prefix.empty())
			str += prefix + ":";

		str += std::to_string(line()) + ":" + std::to_string(column());

		return str;
	}
}
