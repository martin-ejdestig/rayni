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

#ifndef RAYNI_LIB_IO_TEXT_READER_H
#define RAYNI_LIB_IO_TEXT_READER_H

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "lib/system/memory_mapped_file.h"

namespace Rayni
{
	class TextReader
	{
	public:
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

		class Exception : public std::runtime_error
		{
		public:
			using std::runtime_error::runtime_error;

			Exception(const Position &position, const std::string &str) :
			        Exception(position.to_string(), str)
			{
			}

			Exception(const std::string &prefix, const std::string &str) :
			        Exception(prefix.empty() ? str : prefix + ": " + str)
			{
			}
		};

		void open_file(const std::string &file_name);

		void set_string(std::string &&string, const std::string &position_prefix);
		void set_string(std::string &&string)
		{
			set_string(std::move(string), "");
		}

		void close();

		void next()
		{
			if (at_eof())
				throw Exception(position_, "end of stream");

			if (at_newline())
			{
				buffer_position_++;
				position_.next_line();
			}
			else
			{
				buffer_position_++;
				position_.next_column();
			}
		}

		char next_get()
		{
			std::size_t pos = buffer_position_;
			next();
			return buffer_[pos];
		}

		bool at(char c) const
		{
			return !at_eof() && buffer_[buffer_position_] == c;
		}

		bool at_digit() const
		{
			return !at_eof() && buffer_[buffer_position_] >= '0' && buffer_[buffer_position_] <= '9';
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

		bool skip_string(std::string_view str);

		void skip_space()
		{
			while (at_space())
				next();
		}

		std::string_view view(std::size_t start, std::size_t end) const
		{
			assert(start <= buffer_position_);
			assert(end <= buffer_position_);
			assert(start <= end);
			return std::string_view(buffer_ + start, end - start);
		}

		std::size_t buffer_position() const
		{
			return buffer_position_;
		}

		const Position &position() const
		{
			return position_;
		}

	private:
		MemoryMappedFile mmap_file_;
		std::string string_;

		const char *buffer_ = nullptr;
		std::size_t buffer_size_ = 0;
		std::size_t buffer_position_ = 0;

		Position position_;
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
