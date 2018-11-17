/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2018 Martin Ejdestig <marejde@gmail.com>
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

#ifndef RAYNI_LIB_FILE_FORMATS_OBJECT_PLY_READER_H
#define RAYNI_LIB_FILE_FORMATS_OBJECT_PLY_READER_H

#include <string>
#include <vector>

#include "lib/io/binary_type_reader.h"
#include "lib/shapes/triangle_mesh_data.h"

namespace Rayni
{
	class PLYReader : public BinaryTypeReader<TriangleMeshData>
	{
	private:
		enum class Format
		{
			ASCII,
			BINARY_BIG_ENDIAN,
			BINARY_LITTLE_ENDIAN
		};

		enum class BasicType;

		struct Type;
		struct Property;
		struct Element;

		TriangleMeshData read() override;

		std::vector<Element> read_header();
		void read_magic();
		void read_format();
		void skip_comment();

		Element read_element();
		Property read_property(const Element &element);
		Type read_type();

		TriangleMeshData read_mesh_data(const std::vector<Element> &elements);
		void read_vertex_data(const Element &element, TriangleMeshData &data);
		void read_face_data(const Element &element, TriangleMeshData &data);
		void skip_unknown_data(const Element &element);

		template <typename T>
		T read_value(const Type &type);

		template <typename T>
		std::vector<T> read_list_value(const Type &type);

		template <typename T>
		T read_value(BasicType basic_type);

		void skip_value(const Type &type);

		template <typename T>
		T read_number();

		template <typename T>
		T read_ascii_number();

		std::string read_word();
		void skip_word();
		void skip_space();

		Format format_ = Format::ASCII;
	};
}

#endif // RAYNI_LIB_FILE_FORMATS_OBJECT_PLY_READER_H
