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

#include "lib/file_formats/json.h"

#include <string>
#include <utility>

#include "lib/containers/variant.h"
#include "lib/function/result.h"
#include "lib/io/text_reader.h"
#include "lib/string/string.h"

namespace Rayni
{
	namespace
	{
		Result<Variant> read_value(TextReader &reader);

		Result<Variant> read_string(TextReader &reader)
		{
			if (!reader.skip_char('"'))
				return Error(reader.position(), "expected start of string");

			std::string string;

			while (!reader.skip_char('\"')) {
				if (reader.at_newline())
					return Error(reader.position(), "missing string termination");

				if (reader.skip_char('\\')) {
					if (reader.skip_char('b'))
						string += '\b';
					else if (reader.skip_char('t'))
						string += '\t';
					else if (reader.skip_char('n'))
						string += '\n';
					else if (reader.skip_char('f'))
						string += '\f';
					else if (reader.skip_char('r'))
						string += '\r';
					else if (reader.skip_char('\"'))
						string += '\"';
					else if (reader.skip_char('/'))
						string += '/';
					else if (reader.skip_char('\\'))
						string += '\\';
					else if (reader.skip_char('u'))
						return Error(reader.position(),
						             "escaped code points currently not supported");
					else
						return Error(reader.position(), "invalid escape char");
				} else {
					auto c = reader.next_get();
					if (!c)
						return Error(reader.position(), "expected character");
					string += *c;
				}
			}

			return Variant(std::move(string));
		}

		Result<Variant> read_number(TextReader &reader)
		{
			if (!reader.at_digit() && !reader.at('-'))
				return Error(reader.position(), "expected digit or -");

			std::string number;

			if (reader.skip_char('-'))
				number += '-';

			if (reader.skip_char('0')) {
				number += '0';
				if (reader.at_digit())
					return Error(reader.position(), "number may not start with 0");
			} else if (!reader.at_digit())
				return Error(reader.position(), "expected digit between 1-9");

			while (reader.at_digit())
				number += *reader.next_get();

			if (reader.skip_char('.')) {
				number += '.';
				if (!reader.at_digit())
					return Error(reader.position(), "expected digit");
				while (reader.at_digit())
					number += *reader.next_get();
			}

			if (reader.skip_char('e')) {
				number += 'e';
				if (reader.at('-') || reader.at('+'))
					number += *reader.next_get();
				if (!reader.at_digit())
					return Error(reader.position(), "expected digit");
				while (reader.at_digit())
					number += *reader.next_get();
			}

			auto d = string_to_number<double>(number);
			if (!d)
				return Error(reader.position(), "number conversion failed");

			return Variant(*d);
		}

		Result<Variant> read_array(TextReader &reader)
		{
			if (!reader.skip_char('['))
				return Error(reader.position(), "expected start of array");

			Variant::Vector vector;

			reader.skip_space();

			while (!reader.skip_char(']')) {
				Result<Variant> value = read_value(reader);
				if (!value)
					return value.error();
				vector.emplace_back(std::move(*value));

				reader.skip_space();

				if (reader.skip_char(',')) {
					reader.skip_space();
					if (reader.at(']'))
						return Error(reader.position(), "expected value instead of ] after ,");
				} else if (!reader.at(']')) {
					return Error(reader.position(), "expected , or ]");
				}
			}

			return Variant(std::move(vector));
		}

		Result<Variant> read_object(TextReader &reader)
		{
			if (!reader.skip_char('{'))
				return Error(reader.position(), "expected start of object");

			Variant::Map map;

			reader.skip_space();

			while (!reader.skip_char('}')) {
				Result<Variant> key = read_string(reader);
				if (!key)
					return key.error();

				if (map.find(key->as_string()) != map.cend())
					return Error(reader.position(), "duplicate key");

				reader.skip_space();
				if (!reader.skip_char(':'))
					return Error(reader.position(), "expected :");

				Result<Variant> value = read_value(reader);
				if (!value)
					return value.error();
				map.emplace(key->as_string(), std::move(*value));

				reader.skip_space();

				if (reader.skip_char(',')) {
					reader.skip_space();
					if (reader.at('}'))
						return Error(reader.position(), "expected key instead of } after ,");
				} else if (!reader.at('}')) {
					return Error(reader.position(), "expected , or }");
				}
			}

			return Variant(std::move(map));
		}

		Result<Variant> read_value(TextReader &reader)
		{
			reader.skip_space();

			if (reader.at('{'))
				return read_object(reader);

			if (reader.at('['))
				return read_array(reader);

			if (reader.at('"'))
				return read_string(reader);

			if (reader.at_digit() || reader.at('-'))
				return read_number(reader);

			if (reader.skip_string("true"))
				return Variant(true);

			if (reader.skip_string("false"))
				return Variant(false);

			if (reader.skip_string("null"))
				return Variant();

			return Error(reader.position(), "invalid value");
		}

		Result<Variant> read_document(TextReader &reader)
		{
			Result<Variant> value = read_value(reader);
			if (!value)
				return value.error();

			reader.skip_space();

			if (!reader.at_eof())
				return Error(reader.position(), "expected space or end of document");

			return value;
		}
	}

	Result<Variant> json_read_file(const std::string &file_name)
	{
		TextReader reader;
		if (auto r = reader.open_file(file_name); !r)
			return r.error();
		return read_document(reader);
	}

	Result<Variant> json_read_string(std::string &&string)
	{
		TextReader reader;
		reader.set_string(std::move(string));
		return read_document(reader);
	}
}
