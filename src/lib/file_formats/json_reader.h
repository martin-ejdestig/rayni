/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2013-2017 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FILE_FORMATS_JSON_READER_H
#define RAYNI_LIB_FILE_FORMATS_JSON_READER_H

#include "lib/containers/variant.h"
#include "lib/file_formats/text_reader.h"

namespace Rayni
{
	class JSONReader : public TextReader::Parser<Variant>
	{
	private:
		Variant parse() override;

		Variant parse_value();
		Variant parse_object();
		Variant parse_array();
		Variant parse_string();
		Variant parse_number();
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_JSON_READER_H
