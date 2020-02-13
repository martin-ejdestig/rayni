// This file is part of Rayni.
//
// Copyright (C) 2013-2020 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_IO_TEXT_READER_H
#define RAYNI_LIB_IO_TEXT_READER_H

#include <cstddef>
#include <string>
#include <utility>

#include "lib/io/io_exception.h"
#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	class TextReader
	{
	public:
		class Exception;
		class EOFException;

		class Position
		{
		public:
			Position() = default;
			explicit Position(const std::string &prefix);

			void next_line();
			void next_column();
			void next_columns(std::size_t num_columns);

			std::size_t line() const;
			std::size_t column() const;

			std::string to_string() const;

		private:
			bool is_set() const;

			std::size_t line_ = 0;
			std::size_t column_ = 0;
			std::string prefix_;
		};

		void open_file(const std::string &file_name);

		void set_string(std::string &&string, const std::string &position_prefix);
		void set_string(std::string &&string)
		{
			set_string(std::move(string), "");
		}

		void close();

		void next();

		char next_get()
		{
			char c = buffer_[buffer_position_];
			next();
			return c;
		}

		bool at(char c) const
		{
			return buffer_[buffer_position_] == c;
		}

		bool at_digit() const
		{
			char c = buffer_[buffer_position_];
			return c >= '0' && c <= '9';
		}

		bool at_space() const
		{
			return at('\t') || at('\n') || at('\r') || at(' ');
		}

		bool at_newline() const
		{
			return at('\n');
		}

		bool at_eof() const
		{
			return buffer_position_ >= buffer_size_;
		}

		bool skip_char(char c)
		{
			if (!at(c))
				return false;
			next();
			return true;
		}

		bool skip_string(const std::string &str);

		void skip_space()
		{
			while (at_space())
				next();
		}

		void skip_space_on_line()
		{
			while (at_space() && !at_newline())
				next();
		}

		void skip_to_end_of_line()
		{
			while (!at_newline())
				next();
		}

		const Position &position() const
		{
			return position_;
		}

	private:
		MemoryMappedFile mmap_file_;
		std::string string_;

		const char *buffer_ = string_.data();
		std::size_t buffer_size_ = 0;
		std::size_t buffer_position_ = 0;

		Position position_;
	};

	class TextReader::Exception : public IOException
	{
	public:
		using IOException::IOException;

		Exception(const Position &position, const std::string &str) : IOException(position.to_string(), str)
		{
		}
	};

	class TextReader::EOFException : public Exception
	{
	public:
		using Exception::Exception;
	};

	inline TextReader::Position::Position(const std::string &prefix) : prefix_(prefix)
	{
	}

	inline void TextReader::Position::next_line()
	{
		line_++;
		column_ = 1;
	}

	inline void TextReader::Position::next_column()
	{
		column_++;
	}

	inline void TextReader::Position::next_columns(std::size_t num_columns)
	{
		column_ += num_columns;
	}

	inline std::size_t TextReader::Position::line() const
	{
		return line_;
	}

	inline std::size_t TextReader::Position::column() const
	{
		return column_;
	}
}

#endif // RAYNI_LIB_IO_TEXT_READER_H
