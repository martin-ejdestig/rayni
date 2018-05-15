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

#include "lib/file_formats/json_reader.h"

#include <string>
#include <utility>

#include "lib/containers/variant.h"
#include "lib/string/string.h"

namespace Rayni
{
	Variant JSONReader::read()
	{
		Variant value = read_value();

		while (!at_eof())
		{
			if (!at_space())
				throw Exception(position(), "expected space or end of document");

			next();
		}

		return value;
	}

	Variant JSONReader::read_value()
	{
		skip_space();

		if (at('{'))
			return read_object();
		if (at('['))
			return read_array();
		if (at('"'))
			return read_string();
		if (at_digit() || at('-'))
			return read_number();
		if (skip_string("true"))
			return Variant(true);
		if (skip_string("false"))
			return Variant(false);
		if (skip_string("null"))
			return Variant();

		throw Exception(position(), "invalid value");
	}

	Variant JSONReader::read_object()
	{
		if (!skip_char('{'))
			throw Exception(position(), "expected start of object");

		Variant::Map map;

		skip_space();

		while (!skip_char('}'))
		{
			Variant key = read_string();

			if (map.find(key.as_string()) != map.cend())
				throw Exception(position(), "duplicate key");

			skip_space();
			if (!skip_char(':'))
				throw Exception(position(), "expected :");

			map.emplace(key.as_string(), read_value());

			skip_space();

			if (skip_char(','))
			{
				skip_space();
				if (at('}'))
					throw Exception(position(), "expected key instead of } after ,");
			}
			else if (!at('}'))
			{
				throw Exception(position(), "expected , or }");
			}
		}

		return Variant(std::move(map));
	}

	Variant JSONReader::read_array()
	{
		if (!skip_char('['))
			throw Exception(position(), "expected start of array");

		Variant::Vector vector;

		skip_space();

		while (!skip_char(']'))
		{
			vector.emplace_back(read_value());

			skip_space();

			if (skip_char(','))
			{
				skip_space();
				if (at(']'))
					throw Exception(position(), "expected value instead of ] after ,");
			}
			else if (!at(']'))
			{
				throw Exception(position(), "expected , or ]");
			}
		}

		return Variant(std::move(vector));
	}

	Variant JSONReader::read_string()
	{
		if (!skip_char('"'))
			throw Exception(position(), "expected start of string");

		std::string string;

		while (!skip_char('\"'))
		{
			if (at_newline())
				throw Exception(position(), "missing string termination");

			if (skip_char('\\'))
			{
				if (skip_char('b'))
					string += '\b';
				else if (skip_char('t'))
					string += '\t';
				else if (skip_char('n'))
					string += '\n';
				else if (skip_char('f'))
					string += '\f';
				else if (skip_char('r'))
					string += '\r';
				else if (skip_char('\"'))
					string += '\"';
				else if (skip_char('/'))
					string += '/';
				else if (skip_char('\\'))
					string += '\\';
				else if (skip_char('u'))
					throw Exception(position(), "escaped code points currently not supported");
				else
					throw Exception(position(), "invalid escape char");
			}
			else
				string += next_get();
		}

		return Variant(std::move(string));
	}

	Variant JSONReader::read_number()
	{
		if (!at_digit() && !at('-'))
			throw Exception(position(), "expected digit or -");

		std::string number;

		if (at('-'))
			number += next_get();

		if (skip_char('0'))
		{
			number += '0';
			if (at_digit())
				throw Exception(position(), "number may not start with 0");
		}
		else if (!at_digit())
			throw Exception(position(), "expected digit between 1-9");

		while (at_digit())
			number += next_get();

		if (at('.'))
		{
			number += next_get();
			if (!at_digit())
				throw Exception(position(), "expected digit");
			while (at_digit())
				number += next_get();
		}

		if (at('e'))
		{
			number += next_get();
			if (at('-') || at('+'))
				number += next_get();
			if (!at_digit())
				throw Exception(position(), "expected digit");
			while (at_digit())
				number += next_get();
		}

		auto d = string_to_double(number);
		if (!d.has_value())
			throw Exception(position(), "number conversion failed");

		return Variant(d.value());
	}
}
