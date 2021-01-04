// This file is part of Rayni.
//
// Copyright (C) 2018-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/ply.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lib/function/result.h"
#include "lib/io/binary_reader.h"
#include "lib/math/math.h"
#include "lib/math/numeric_cast.h"
#include "lib/math/vector3.h"
#include "lib/string/string.h"

namespace Rayni
{
	namespace
	{
		enum class Format
		{
			ASCII,
			BINARY_BIG_ENDIAN,
			BINARY_LITTLE_ENDIAN
		};

		enum class BasicType
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

		struct Type
		{
			BasicType basic_type = BasicType::INT8;
			BasicType list_size_type = BasicType::INT8;
			bool is_list = false;
		};

		struct Property
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

		struct Element
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
				for (const auto &p : element.properties)
					if (p.name == property_name)
						return true;
				return false;
			}

			Name name = Name::UNKNOWN;
			Count count = 0;
			std::vector<Property> properties;
		};

		struct Header
		{
			static bool has_element(const Header &header, Element::Name name)
			{
				for (const auto &e : header.elements)
					if (e.name == name)
						return true;
				return false;
			}

			Format format = Format::ASCII;
			std::vector<Element> elements;
		};

		void skip_space(BinaryReader &reader)
		{
			while (!reader.at_eof()) {
				if (!reader.at(' ') && !reader.at('\n'))
					break;

				[[maybe_unused]] auto r = reader.skip_bytes(1);
				assert(r);
			}
		}

		void skip_comment(BinaryReader &reader)
		{
			while (!reader.at_eof()) {
				std::int8_t c = *reader.read_int8();

				if (c == '\n')
					break;
			}
		}

		void skip_word(BinaryReader &reader)
		{
			skip_space(reader);

			while (!reader.at_eof()) {
				std::int8_t c = *reader.read_int8();

				if (c == ' ' || c == '\n')
					break;
			}
		}

		std::string read_word(BinaryReader &reader)
		{
			std::string word;

			skip_space(reader);

			while (!reader.at_eof()) {
				std::int8_t c = *reader.read_int8();

				if (c == ' ' || c == '\n')
					break;

				word += c;
			}

			return word;
		}

		Result<Type> read_type(BinaryReader &reader)
		{
			auto str_to_basic_type = [&](const std::string &str) -> Result<BasicType> {
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

				return Error(reader.position(), "unknown type \"" + str + "\"");
			};

			std::string str = read_word(reader);
			Type type;

			if (str != "list") {
				if (auto r = str_to_basic_type(str); r)
					type.basic_type = *r;
				else
					return r.error();
			} else {
				if (auto r = str_to_basic_type(read_word(reader)); r)
					type.list_size_type = *r;
				else
					return r.error();

				if (auto r = str_to_basic_type(read_word(reader)); r)
					type.basic_type = *r;
				else
					return r.error();

				type.is_list = true;
			}

			return type;
		}

		template <typename T>
		Result<T> read_ascii_number(BinaryReader &reader)
		{
			auto v = string_to_number<T>(read_word(reader));

			if (!v)
				return Error(reader.position(), "invalid ASCII number");

			return Result<T>(std::move(*v));
		}

		template <typename T>
		Result<T> read_number(BinaryReader &reader, const Header &header)
		{
			if (header.format == Format::BINARY_BIG_ENDIAN)
				return reader.read_big_endian<T>();
			if (header.format == Format::BINARY_LITTLE_ENDIAN)
				return reader.read_little_endian<T>();

			return read_ascii_number<T>(reader); // header.format == Format::ASCII
		}

		template <typename T>
		Result<T> read_number(BinaryReader &reader, const Header &header, BasicType basic_type)
		{
			auto cast = [&](auto r) -> Result<T> {
				if (!r)
					return r.error();
				std::optional<T> v = numeric_cast<T>(*r);
				if (!v)
					return Error(reader.position(), "value out of range");
				return Result<T>(std::move(*v));
			};

			switch (basic_type) {
			case BasicType::INT8:
				return cast(read_number<std::int8_t>(reader, header));
			case BasicType::UINT8:
				return cast(read_number<std::uint8_t>(reader, header));
			case BasicType::INT16:
				return cast(read_number<std::int16_t>(reader, header));
			case BasicType::UINT16:
				return cast(read_number<std::uint16_t>(reader, header));
			case BasicType::INT32:
				return cast(read_number<std::int32_t>(reader, header));
			case BasicType::UINT32:
				return cast(read_number<std::uint32_t>(reader, header));
			case BasicType::FLOAT32:
				return cast(read_number<float>(reader, header));
			case BasicType::FLOAT64:
				return cast(read_number<double>(reader, header));
			}

			assert(false);
			return Error(reader.position(), "unhandled number type");
		}

		template <typename T>
		Result<T> read_number(BinaryReader &reader, const Header &header, const Type &type)
		{
			if (type.is_list)
				return Error(reader.position(), "unexpected list value, expected scalar");

			return read_number<T>(reader, header, type.basic_type);
		}

		template <typename T>
		Result<void> read_list(BinaryReader &reader,
		                       const Header &header,
		                       const Type &type,
		                       std::vector<T> &dest)
		{
			if (!type.is_list)
				return Error(reader.position(), "unexpected scalar value, expected list");

			std::uint32_t count;
			if (auto r = read_number<std::uint32_t>(reader, header, type.list_size_type); r)
				count = *r;
			else
				return r.error();

			dest.clear(); // Assume capacity() is left unchanged and memory is reused.
			dest.reserve(count);

			for (std::uint32_t i = 0; i < count; i++) {
				auto n = read_number<T>(reader, header, type.basic_type);
				if (!n)
					return n.error();
				dest.emplace_back(*n);
			}

			return {};
		}

		Result<void> skip_value(BinaryReader &reader, const Header &header, const Type &type)
		{
			std::size_t count = 1;

			if (type.is_list) {
				if (auto r = read_number<std::uint32_t>(reader, header, type.list_size_type); r)
					count = *r;
				else
					return r.error();
			}

			if (header.format == Format::ASCII) {
				for (std::size_t i = 0; i < count; i++)
					skip_word(reader);
			} else {
				std::size_t basic_type_size = 0;

				switch (type.basic_type) {
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

				if (auto r = reader.skip_bytes(basic_type_size * count); !r)
					return r.error();
			}

			return {};
		}

		Result<void> read_magic(BinaryReader &reader)
		{
			const std::array<std::uint8_t, 4> expected_magic = {'p', 'l', 'y', '\n'};
			std::array<std::uint8_t, 4> magic;

			if (auto r = reader.read_bytes(magic); !r)
				return r.error();

			if (magic != expected_magic)
				return Error(reader.position(), R"(header must start with "ply\n")");

			return {};
		}

		Result<Format> read_format(BinaryReader &reader)
		{
			if (read_word(reader) != "format")
				return Error(reader.position(), "expected \"format\"");

			std::string format_str = read_word(reader);
			Format format;

			if (format_str == "ascii")
				format = Format::ASCII;
			else if (format_str == "binary_big_endian")
				format = Format::BINARY_BIG_ENDIAN;
			else if (format_str == "binary_little_endian")
				format = Format::BINARY_LITTLE_ENDIAN;
			else
				return Error(reader.position(), "unknown format type: \"" + format_str + "\"");

			std::string version = read_word(reader);

			if (version != "1.0")
				return Error(reader.position(), "unknown version: \"" + version + "\"");

			return format;
		}

		Result<Element> read_element(BinaryReader &reader)
		{
			Element element;

			std::string name = read_word(reader);
			if (name.empty())
				return Error(reader.position(), "expected element name");

			if (name == "vertex")
				element.name = Element::Name::VERTEX;
			else if (name == "face")
				element.name = Element::Name::FACE;

			if (auto r = read_ascii_number<Element::Count>(reader); r)
				element.count = *r;
			else
				return r.error();

			return element;
		}

		Result<Property> read_property(BinaryReader &reader, Element::Name element_name)
		{
			Property property;

			if (auto r = read_type(reader); r)
				property.type = *r;
			else
				return r.error();

			std::string name = read_word(reader);
			if (name.empty())
				return Error(reader.position(), "expected property name");

			if (element_name == Element::Name::VERTEX) {
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
			} else if (element_name == Element::Name::FACE) {
				if (name == "vertex_indices")
					property.name = Property::Name::VERTEX_INDICES;
			}

			return property;
		}

		Result<Header> read_header(BinaryReader &reader)
		{
			Header header;

			if (auto r = read_magic(reader); !r)
				return r.error();

			if (auto r = read_format(reader); r)
				header.format = *r;
			else
				return r.error();

			while (true) {
				std::string keyword = read_word(reader);

				if (keyword == "comment") {
					skip_comment(reader);
				} else if (keyword == "element") {
					Result<Element> element = read_element(reader);
					if (!element)
						return element.error();

					if (element->name != Element::Name::UNKNOWN &&
					    Header::has_element(header, element->name))
						return Error(reader.position(), "duplicate element in header");

					header.elements.emplace_back(*element);
				} else if (keyword == "property") {
					if (header.elements.empty())
						return Error(reader.position(),
						             "property found in header before any element");

					Element &element = header.elements.back();
					Result<Property> property = read_property(reader, element.name);
					if (!property)
						return property.error();

					if (property->name != Property::Name::UNKNOWN &&
					    Element::has_property(element, property->name))
						return Error(reader.position(), "duplicate property for element");

					element.properties.emplace_back(*property);
				} else if (keyword == "end_header") {
					break;
				} else {
					return Error(reader.position(), "unknown header keyword: \"" + keyword + "\"");
				}
			}

			if (!Header::has_element(header, Element::Name::VERTEX))
				return Error(reader.position(), "missing vertex element in header");

			if (!Header::has_element(header, Element::Name::FACE))
				return Error(reader.position(), "missing face element in header");

			for (const auto &element : header.elements)
				if (element.properties.empty())
					return Error(reader.position(), "element without properties in header");

			return header;
		}

		Result<void> read_vertex_data(BinaryReader &reader,
		                              const Header &header,
		                              const Element &element,
		                              TriangleMeshData &data)
		{
			bool has_uvs = Element::has_property(element, Property::Name::VERTEX_U) ||
			               Element::has_property(element, Property::Name::VERTEX_V);
			bool has_normals = Element::has_property(element, Property::Name::VERTEX_NORMAL_X) ||
			                   Element::has_property(element, Property::Name::VERTEX_NORMAL_Y) ||
			                   Element::has_property(element, Property::Name::VERTEX_NORMAL_Z);
			Vector3 point;
			Vector3 normal;
			TriangleMeshData::UV uv;

			data.points.reserve(element.count);
			if (has_normals)
				data.normals.reserve(element.count);
			if (has_uvs)
				data.uvs.reserve(element.count);

			for (Element::Count i = 0; i < element.count; i++) {
				for (const Property &property : element.properties) {
					Result<real_t> n = read_number<real_t>(reader, header, property.type);
					if (!n)
						return n.error();

					if (property.name == Property::Name::VERTEX_X)
						point.x() = *n;
					else if (property.name == Property::Name::VERTEX_Y)
						point.y() = *n;
					else if (property.name == Property::Name::VERTEX_Z)
						point.z() = *n;
					else if (property.name == Property::Name::VERTEX_NORMAL_X)
						normal.x() = *n;
					else if (property.name == Property::Name::VERTEX_NORMAL_Y)
						normal.y() = *n;
					else if (property.name == Property::Name::VERTEX_NORMAL_Z)
						normal.z() = *n;
					else if (property.name == Property::Name::VERTEX_U)
						uv.u = *n;
					else if (property.name == Property::Name::VERTEX_V)
						uv.v = *n;
				}

				data.points.emplace_back(point);
				if (has_normals)
					data.normals.emplace_back(normal);
				if (has_uvs)
					data.uvs.emplace_back(uv);
			}

			return {};
		}

		Result<void> read_face_data(BinaryReader &reader,
		                            const Header &header,
		                            const Element &element,
		                            TriangleMeshData &data)
		{
			std::vector<TriangleMeshData::Index> indices;

			data.indices.reserve(element.count);

			for (Element::Count i = 0; i < element.count; i++) {
				for (const Property &property : element.properties) {
					if (property.name == Property::Name::VERTEX_INDICES) {
						if (auto r = read_list<TriangleMeshData::Index>(reader,
						                                                header,
						                                                property.type,
						                                                indices);
						    !r)
							return r.error();

						if (indices.size() < 3)
							return Error(reader.position(),
							             "face element must have at least 3 indices");

						TriangleMeshData::Index i1 = indices[0];
						TriangleMeshData::Index i2 = 0;
						TriangleMeshData::Index i3 = indices[1];

						for (std::size_t j = 2; j < indices.size(); j++) {
							i2 = i3;
							i3 = indices[j];
							data.indices.emplace_back(i1, i2, i3);
						}
					} else {
						if (auto r = skip_value(reader, header, property.type); !r)
							return r.error();
					}
				}
			}

			return {};
		}

		Result<TriangleMeshData> read_mesh_data(BinaryReader &reader, const Header &header)
		{
			TriangleMeshData data;

			for (const Element &element : header.elements) {
				if (element.name == Element::Name::VERTEX) {
					if (auto r = read_vertex_data(reader, header, element, data); !r)
						return r.error();
				} else if (element.name == Element::Name::FACE) {
					if (auto r = read_face_data(reader, header, element, data); !r)
						return r.error();
				} else {
					for (Element::Count i = 0; i < element.count; i++)
						for (const Property &property : element.properties)
							if (auto r = skip_value(reader, header, property.type); !r)
								return r.error();
				}
			}

			if (data.points.size() < 3)
				return Error(reader.position(), "number of vertices must be at least 3");

			if (data.indices.empty())
				return Error(reader.position(), "missing indices");

			auto max_index = data.points.size() - 1;

			for (auto &indices : data.indices)
				if (indices.index1 > max_index || indices.index2 > max_index ||
				    indices.index3 > max_index)
					return Error(reader.position(),
					             "invalid indices (" + std::to_string(indices.index1) + ", " +
					                     std::to_string(indices.index2) + ", " +
					                     std::to_string(indices.index3) + ")" +
					                     ", max allowed: " + std::to_string(max_index));

			return data;
		}

		Result<TriangleMeshData> read_ply(BinaryReader &reader)
		{
			Result<Header> header = read_header(reader);
			if (!header)
				return header.error();

			return read_mesh_data(reader, *header);
		}
	}

	Result<TriangleMeshData> ply_read_file(const std::string &file_name)
	{
		BinaryReader reader;
		if (auto r = reader.open_file(file_name); !r)
			return r.error();
		return read_ply(reader);
	}

	Result<TriangleMeshData> ply_read_data(std::vector<std::uint8_t> &&data)
	{
		BinaryReader reader;
		reader.set_data(std::move(data));
		return read_ply(reader);
	}
}
