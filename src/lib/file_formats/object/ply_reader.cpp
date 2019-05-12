// This file is part of Rayni.
//
// Copyright (C) 2018-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/object/ply_reader.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lib/math/math.h"
#include "lib/math/numeric_cast.h"
#include "lib/math/vector3.h"
#include "lib/string/string.h"

namespace Rayni
{
	enum class PLYReader::BasicType
	{
		INT8,
		UINT8,
		INT16,
		UINT16,
		INT32,
		UINT32,
		FLOAT32,
		FLOAT64
	};

	struct PLYReader::Type
	{
		BasicType basic_type = BasicType::INT8;
		BasicType list_size_type = BasicType::INT8;
		bool is_list = false;
	};

	struct PLYReader::Property
	{
		enum class Name
		{
			UNKNOWN,

			VERTEX_X,
			VERTEX_Y,
			VERTEX_Z,
			VERTEX_NORMAL_X,
			VERTEX_NORMAL_Y,
			VERTEX_NORMAL_Z,
			VERTEX_U,
			VERTEX_V,

			VERTEX_INDICES
		};

		Type type;
		Name name = Name::UNKNOWN;
	};

	struct PLYReader::Element
	{
		enum class Name
		{
			UNKNOWN,
			VERTEX,
			FACE
		};

		using Count = std::uint32_t;

		static bool has_property(const Element &element, Property::Name property_name)
		{
			for (auto &p : element.properties)
				if (p.name == property_name)
					return true;
			return false;
		}

		Name name = Name::UNKNOWN;
		Count count = 0;
		std::vector<Property> properties;
	};

	TriangleMeshData PLYReader::read()
	{
		std::vector<Element> elements = read_header();

		return read_mesh_data(elements);
	}

	std::vector<PLYReader::Element> PLYReader::read_header()
	{
		read_magic();
		read_format();

		std::vector<Element> elements;
		auto elements_has = [&](Element::Name name) {
			for (const auto &e : elements)
				if (e.name == name)
					return true;
			return false;
		};

		while (true)
		{
			std::string keyword = read_word();

			if (keyword == "comment")
			{
				skip_comment();
			}
			else if (keyword == "element")
			{
				Element element = read_element();

				if (element.name != Element::Name::UNKNOWN && elements_has(element.name))
					throw Exception(position(), "duplicate element in header");

				elements.emplace_back(element);
			}
			else if (keyword == "property")
			{
				if (elements.empty())
					throw Exception(position(), "property found in header before any element");

				Element &element = elements.back();
				Property property = read_property(element);

				if (property.name != Property::Name::UNKNOWN &&
				    Element::has_property(element, property.name))
					throw Exception(position(), "duplicate property for element");

				element.properties.emplace_back(property);
			}
			else if (keyword == "end_header")
			{
				break;
			}
			else
			{
				throw Exception(position(), "unknown header keyword: \"" + keyword + "\"");
			}
		}

		if (!elements_has(Element::Name::VERTEX))
			throw Exception(position(), "missing vertex element in header");

		if (!elements_has(Element::Name::FACE))
			throw Exception(position(), "missing face element in header");

		for (const auto &element : elements)
			if (element.properties.empty())
				throw Exception(position(), "element without properties in header");

		return elements;
	}

	void PLYReader::read_magic()
	{
		const std::array<std::uint8_t, 4> expected_magic = {'p', 'l', 'y', '\n'};
		std::array<std::uint8_t, 4> magic;

		read_bytes(magic);

		if (magic != expected_magic)
			throw Exception(position(), R"(header must start with "ply\n")");
	}

	void PLYReader::read_format()
	{
		if (read_word() != "format")
			throw Exception(position(), "expected \"format\"");

		std::string format_str = read_word();

		if (format_str == "ascii")
			format_ = Format::ASCII;
		else if (format_str == "binary_big_endian")
			format_ = Format::BINARY_BIG_ENDIAN;
		else if (format_str == "binary_little_endian")
			format_ = Format::BINARY_LITTLE_ENDIAN;
		else
			throw Exception(position(), "unknown format type: \"" + format_str + "\"");

		std::string version = read_word();

		if (version != "1.0")
			throw Exception(position(), "unknown version: \"" + version + "\"");
	}

	void PLYReader::skip_comment()
	{
		while (read_int8() != '\n')
			;
	}

	PLYReader::Element PLYReader::read_element()
	{
		Element element;

		std::string name = read_word();
		if (name == "vertex")
			element.name = Element::Name::VERTEX;
		else if (name == "face")
			element.name = Element::Name::FACE;

		element.count = read_ascii_number<Element::Count>();

		return element;
	}

	PLYReader::Property PLYReader::read_property(const Element &element)
	{
		Property property;

		property.type = read_type();
		std::string name = read_word();

		if (element.name == Element::Name::VERTEX)
		{
			// Only x, y, z is mentioned in original spec. Have seen all others below
			// in .ply files though. Annoying with different names for same properties.
			if (name == "x")
				property.name = Property::Name::VERTEX_X;
			else if (name == "y")
				property.name = Property::Name::VERTEX_Y;
			else if (name == "z")
				property.name = Property::Name::VERTEX_Z;
			else if (name == "nx")
				property.name = Property::Name::VERTEX_NORMAL_X;
			else if (name == "ny")
				property.name = Property::Name::VERTEX_NORMAL_Y;
			else if (name == "nz")
				property.name = Property::Name::VERTEX_NORMAL_Z;
			else if (name == "u" || name == "s" || name == "texture_u" || name == "texture_s")
				property.name = Property::Name::VERTEX_U;
			else if (name == "v" || name == "t" || name == "texture_v" || name == "texture_t")
				property.name = Property::Name::VERTEX_V;
		}
		else if (element.name == Element::Name::FACE)
		{
			if (name == "vertex_indices")
				property.name = Property::Name::VERTEX_INDICES;
		}

		return property;
	}

	PLYReader::Type PLYReader::read_type()
	{
		auto str_to_basic_type = [&](const std::string &str) {
			if (str == "int8" || str == "char")
				return BasicType::INT8;
			if (str == "uint8" || str == "uchar")
				return BasicType::UINT8;
			if (str == "int16" || str == "short")
				return BasicType::INT16;
			if (str == "uint16" || str == "ushort")
				return BasicType::UINT16;
			if (str == "int32" || str == "int")
				return BasicType::INT32;
			if (str == "uint32" || str == "uint")
				return BasicType::UINT32;
			if (str == "float32" || str == "float")
				return BasicType::FLOAT32;
			if (str == "float64" || str == "double")
				return BasicType::FLOAT64;

			throw Exception(position(), "unknown type \"" + str + "\"");
		};

		std::string str = read_word();
		Type type;

		if (str != "list")
		{
			type.basic_type = str_to_basic_type(str);
		}
		else
		{
			type.list_size_type = str_to_basic_type(read_word());
			type.basic_type = str_to_basic_type(read_word());
			type.is_list = true;
		}

		return type;
	}

	TriangleMeshData PLYReader::read_mesh_data(const std::vector<Element> &elements)
	{
		TriangleMeshData data;

		for (const Element &element : elements)
		{
			if (element.name == Element::Name::VERTEX)
				read_vertex_data(element, data);
			else if (element.name == Element::Name::FACE)
				read_face_data(element, data);
			else
				skip_unknown_data(element);
		}

		if (data.points.size() < 3)
			throw Exception(position(), "number of vertices must be at least 3");

		if (data.indices.empty())
			throw Exception(position(), "missing indices");

		auto max_index = data.points.size() - 1;

		for (auto &indices : data.indices)
			if (indices.index1 > max_index || indices.index2 > max_index || indices.index3 > max_index)
				throw Exception(position(),
				                "invalid indices (" + std::to_string(indices.index1) + ", " +
				                        std::to_string(indices.index2) + ", " +
				                        std::to_string(indices.index3) + ")" +
				                        ", max allowed: " + std::to_string(max_index));

		return data;
	}

	void PLYReader::read_vertex_data(const Element &element, TriangleMeshData &data)
	{
		bool has_uvs = Element::has_property(element, Property::Name::VERTEX_U) ||
		               Element::has_property(element, Property::Name::VERTEX_V);
		bool has_normals = Element::has_property(element, Property::Name::VERTEX_NORMAL_X) ||
		                   Element::has_property(element, Property::Name::VERTEX_NORMAL_Y) ||
		                   Element::has_property(element, Property::Name::VERTEX_NORMAL_Z);
		Vector3 point;
		Vector3 normal;
		TriangleMeshData::UV uv;

		for (Element::Count i = 0; i < element.count; i++)
		{
			for (const Property &property : element.properties)
			{
				if (property.name == Property::Name::VERTEX_X)
					point.x() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_Y)
					point.y() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_Z)
					point.z() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_NORMAL_X)
					normal.x() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_NORMAL_Y)
					normal.y() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_NORMAL_Z)
					normal.z() = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_U)
					uv.u = read_value<real_t>(property.type);
				else if (property.name == Property::Name::VERTEX_V)
					uv.v = read_value<real_t>(property.type);
				else
					skip_value(property.type);
			}

			data.points.emplace_back(point);
			if (has_normals)
				data.normals.emplace_back(normal);
			if (has_uvs)
				data.uvs.emplace_back(uv);
		}
	}

	void PLYReader::read_face_data(const Element &element, TriangleMeshData &data)
	{
		auto read_indices = [&](const Type &type) {
			std::vector<TriangleMeshData::Index> indices = read_list_value<TriangleMeshData::Index>(type);

			if (indices.size() < 3)
				throw Exception(position(), "face element must have at least 3 indices");

			TriangleMeshData::Index i1 = indices[0];
			TriangleMeshData::Index i2 = 0;
			TriangleMeshData::Index i3 = indices[1];

			for (std::size_t i = 2; i < indices.size(); i++)
			{
				i2 = i3;
				i3 = indices[i];
				data.indices.emplace_back(i1, i2, i3);
			}
		};

		for (Element::Count i = 0; i < element.count; i++)
		{
			for (const Property &property : element.properties)
			{
				if (property.name == Property::Name::VERTEX_INDICES)
					read_indices(property.type);
				else
					skip_value(property.type);
			}
		}
	}

	void PLYReader::skip_unknown_data(const Element &element)
	{
		for (Element::Count i = 0; i < element.count; i++)
			for (const Property &property : element.properties)
				skip_value(property.type);
	}

	template <typename T>
	T PLYReader::read_value(const Type &type)
	{
		if (type.is_list)
			throw Exception(position(), "unexpected list value, expected scalar");

		return read_value<T>(type.basic_type);
	}

	template <typename T>
	std::vector<T> PLYReader::read_list_value(const Type &type)
	{
		if (!type.is_list)
			throw Exception(position(), "unexpected scalar value, expected list");

		std::size_t count = read_value<std::uint32_t>(type.list_size_type);
		std::vector<T> v;
		v.reserve(count);

		for (std::size_t i = 0; i < count; i++)
			v.emplace_back(read_value<T>(type.basic_type));

		return v;
	}

	template <typename T>
	T PLYReader::read_value(BasicType basic_type)
	{
		std::optional<T> value = 0;

		switch (basic_type)
		{
		case BasicType::INT8:
			value = numeric_cast<T>(read_number<std::int8_t>());
			break;
		case BasicType::UINT8:
			value = numeric_cast<T>(read_number<std::uint8_t>());
			break;
		case BasicType::INT16:
			value = numeric_cast<T>(read_number<std::int16_t>());
			break;
		case BasicType::UINT16:
			value = numeric_cast<T>(read_number<std::uint16_t>());
			break;
		case BasicType::INT32:
			value = numeric_cast<T>(read_number<std::int32_t>());
			break;
		case BasicType::UINT32:
			value = numeric_cast<T>(read_number<std::uint32_t>());
			break;
		case BasicType::FLOAT32:
			value = numeric_cast<T>(read_number<float>());
			break;
		case BasicType::FLOAT64:
			value = numeric_cast<T>(read_number<double>());
			break;
		}

		if (!value)
			throw Exception(position(), "value out of range");

		return *value;
	}

	void PLYReader::skip_value(const Type &type)
	{
		std::size_t count = 1;

		if (type.is_list)
			count = read_value<std::uint32_t>(type.list_size_type);

		if (format_ == Format::ASCII)
		{
			for (std::size_t i = 0; i < count; i++)
				skip_word();
		}
		else
		{
			std::size_t basic_type_size = 0;

			switch (type.basic_type)
			{
			case BasicType::INT8:
			case BasicType::UINT8:
				basic_type_size = 1;
				break;
			case BasicType::INT16:
			case BasicType::UINT16:
				basic_type_size = 2;
				break;
			case BasicType::INT32:
			case BasicType::UINT32:
			case BasicType::FLOAT32:
				basic_type_size = 4;
				break;
			case BasicType::FLOAT64:
				basic_type_size = 8;
				break;
			}

			skip_bytes(basic_type_size * count);
		}
	}

	template <typename T>
	T PLYReader::read_number()
	{
		if (format_ == Format::ASCII)
			return read_ascii_number<T>();

		return format_ == Format::BINARY_BIG_ENDIAN ? read_big_endian<T>() : read_little_endian<T>();
	}

	template <typename T>
	T PLYReader::read_ascii_number()
	{
		auto v = string_to_number<T>(read_word());

		if (!v.has_value())
			throw Exception(position(), "failed to convert string to number");

		return v.value();
	}

	std::string PLYReader::read_word()
	{
		std::string word;

		skip_space();

		while (true)
		{
			std::int8_t c = read_int8();

			if (c == ' ' || c == '\n')
				break;

			word += c;
		}

		return word;
	}

	void PLYReader::skip_word()
	{
		skip_space();

		while (true)
		{
			std::int8_t c = read_int8();

			if (c == ' ' || c == '\n')
				break;
		}
	}

	void PLYReader::skip_space()
	{
		while (true)
		{
			std::optional<std::int8_t> c = peek_int8();

			if (!c.has_value() || (c.value() != ' ' && c.value() != '\n'))
				break;

			read_int8();
		}
	}
}
