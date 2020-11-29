// This file is part of Rayni.
//
// Copyright (C) 2018-2020 Martin Ejdestig <marejde@gmail.com>
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
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lib/io/binary_reader.h"
#include "lib/math/math.h"
#include "lib/math/numeric_cast.h"
#include "lib/math/vector3.h"
#include "lib/string/string.h"

namespace Rayni
{
	namespace
	{
		using Exception = BinaryReader::Exception;

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
			while (true)
			{
				std::optional<std::int8_t> c = reader.peek_int8();

				if (!c.has_value() || (c.value() != ' ' && c.value() != '\n'))
					break;

				reader.read_int8();
			}
		}

		void skip_comment(BinaryReader &reader)
		{
			while (reader.read_int8() != '\n')
				;
		}

		void skip_word(BinaryReader &reader)
		{
			skip_space(reader);

			while (true)
			{
				std::int8_t c = reader.read_int8();

				if (c == ' ' || c == '\n')
					break;
			}
		}

		std::string read_word(BinaryReader &reader)
		{
			std::string word;

			skip_space(reader);

			while (true)
			{
				std::int8_t c = reader.read_int8();

				if (c == ' ' || c == '\n')
					break;

				word += c;
			}

			return word;
		}

		Type read_type(BinaryReader &reader)
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

				throw Exception(reader.position(), "unknown type \"" + str + "\"");
			};

			std::string str = read_word(reader);
			Type type;

			if (str != "list")
			{
				type.basic_type = str_to_basic_type(str);
			}
			else
			{
				type.list_size_type = str_to_basic_type(read_word(reader));
				type.basic_type = str_to_basic_type(read_word(reader));
				type.is_list = true;
			}

			return type;
		}

		template <typename T>
		T read_ascii_number(BinaryReader &reader)
		{
			auto v = string_to_number<T>(read_word(reader));

			if (!v.has_value())
				throw Exception(reader.position(), "failed to convert string to number");

			return v.value();
		}

		template <typename T>
		T read_number(BinaryReader &reader, const Header &header)
		{
			if (header.format == Format::ASCII)
				return read_ascii_number<T>(reader);

			return header.format == Format::BINARY_BIG_ENDIAN ? reader.read_big_endian<T>() :
                                                                            reader.read_little_endian<T>();
		}

		template <typename T>
		T read_number(BinaryReader &reader, const Header &header, BasicType basic_type)
		{
			std::optional<T> value = 0;

			switch (basic_type)
			{
			case BasicType::INT8:
				value = numeric_cast<T>(read_number<std::int8_t>(reader, header));
				break;
			case BasicType::UINT8:
				value = numeric_cast<T>(read_number<std::uint8_t>(reader, header));
				break;
			case BasicType::INT16:
				value = numeric_cast<T>(read_number<std::int16_t>(reader, header));
				break;
			case BasicType::UINT16:
				value = numeric_cast<T>(read_number<std::uint16_t>(reader, header));
				break;
			case BasicType::INT32:
				value = numeric_cast<T>(read_number<std::int32_t>(reader, header));
				break;
			case BasicType::UINT32:
				value = numeric_cast<T>(read_number<std::uint32_t>(reader, header));
				break;
			case BasicType::FLOAT32:
				value = numeric_cast<T>(read_number<float>(reader, header));
				break;
			case BasicType::FLOAT64:
				value = numeric_cast<T>(read_number<double>(reader, header));
				break;
			}

			if (!value)
				throw Exception(reader.position(), "value out of range");

			return *value;
		}

		template <typename T>
		T read_number(BinaryReader &reader, const Header &header, const Type &type)
		{
			if (type.is_list)
				throw Exception(reader.position(), "unexpected list value, expected scalar");

			return read_number<T>(reader, header, type.basic_type);
		}

		template <typename T>
		void read_list(BinaryReader &reader, const Header &header, const Type &type, std::vector<T> &dest)
		{
			if (!type.is_list)
				throw Exception(reader.position(), "unexpected scalar value, expected list");

			std::size_t count = read_number<std::uint32_t>(reader, header, type.list_size_type);
			dest.clear(); // Assume capacity() is left unchanged and memory is reused.
			dest.reserve(count);

			for (std::size_t i = 0; i < count; i++)
				dest.emplace_back(read_number<T>(reader, header, type.basic_type));
		}

		void skip_value(BinaryReader &reader, const Header &header, const Type &type)
		{
			std::size_t count = 1;

			if (type.is_list)
				count = read_number<std::uint32_t>(reader, header, type.list_size_type);

			if (header.format == Format::ASCII)
			{
				for (std::size_t i = 0; i < count; i++)
					skip_word(reader);
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

				reader.skip_bytes(basic_type_size * count);
			}
		}

		void read_magic(BinaryReader &reader)
		{
			const std::array<std::uint8_t, 4> expected_magic = {'p', 'l', 'y', '\n'};
			std::array<std::uint8_t, 4> magic;

			reader.read_bytes(magic);

			if (magic != expected_magic)
				throw Exception(reader.position(), R"(header must start with "ply\n")");
		}

		Format read_format(BinaryReader &reader)
		{
			if (read_word(reader) != "format")
				throw Exception(reader.position(), "expected \"format\"");

			std::string format_str = read_word(reader);
			Format format;

			if (format_str == "ascii")
				format = Format::ASCII;
			else if (format_str == "binary_big_endian")
				format = Format::BINARY_BIG_ENDIAN;
			else if (format_str == "binary_little_endian")
				format = Format::BINARY_LITTLE_ENDIAN;
			else
				throw Exception(reader.position(), "unknown format type: \"" + format_str + "\"");

			std::string version = read_word(reader);

			if (version != "1.0")
				throw Exception(reader.position(), "unknown version: \"" + version + "\"");

			return format;
		}

		Element read_element(BinaryReader &reader)
		{
			Element element;

			std::string name = read_word(reader);
			if (name == "vertex")
				element.name = Element::Name::VERTEX;
			else if (name == "face")
				element.name = Element::Name::FACE;

			element.count = read_ascii_number<Element::Count>(reader);

			return element;
		}

		Property read_property(BinaryReader &reader, Element::Name element_name)
		{
			Property property;

			property.type = read_type(reader);
			std::string name = read_word(reader);

			if (element_name == Element::Name::VERTEX)
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
			else if (element_name == Element::Name::FACE)
			{
				if (name == "vertex_indices")
					property.name = Property::Name::VERTEX_INDICES;
			}

			return property;
		}

		Header read_header(BinaryReader &reader)
		{
			Header header;

			read_magic(reader);
			header.format = read_format(reader);

			while (true)
			{
				std::string keyword = read_word(reader);

				if (keyword == "comment")
				{
					skip_comment(reader);
				}
				else if (keyword == "element")
				{
					Element element = read_element(reader);

					if (element.name != Element::Name::UNKNOWN &&
					    Header::has_element(header, element.name))
						throw Exception(reader.position(), "duplicate element in header");

					header.elements.emplace_back(element);
				}
				else if (keyword == "property")
				{
					if (header.elements.empty())
						throw Exception(reader.position(),
						                "property found in header before any element");

					Element &element = header.elements.back();
					Property property = read_property(reader, element.name);

					if (property.name != Property::Name::UNKNOWN &&
					    Element::has_property(element, property.name))
						throw Exception(reader.position(), "duplicate property for element");

					element.properties.emplace_back(property);
				}
				else if (keyword == "end_header")
				{
					break;
				}
				else
				{
					throw Exception(reader.position(),
					                "unknown header keyword: \"" + keyword + "\"");
				}
			}

			if (!Header::has_element(header, Element::Name::VERTEX))
				throw Exception(reader.position(), "missing vertex element in header");

			if (!Header::has_element(header, Element::Name::FACE))
				throw Exception(reader.position(), "missing face element in header");

			for (const auto &element : header.elements)
				if (element.properties.empty())
					throw Exception(reader.position(), "element without properties in header");

			return header;
		}

		void read_vertex_data(BinaryReader &reader,
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

			for (Element::Count i = 0; i < element.count; i++)
			{
				for (const Property &property : element.properties)
				{
					if (property.name == Property::Name::VERTEX_X)
						point.x() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_Y)
						point.y() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_Z)
						point.z() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_NORMAL_X)
						normal.x() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_NORMAL_Y)
						normal.y() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_NORMAL_Z)
						normal.z() = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_U)
						uv.u = read_number<real_t>(reader, header, property.type);
					else if (property.name == Property::Name::VERTEX_V)
						uv.v = read_number<real_t>(reader, header, property.type);
					else
						skip_value(reader, header, property.type);
				}

				data.points.emplace_back(point);
				if (has_normals)
					data.normals.emplace_back(normal);
				if (has_uvs)
					data.uvs.emplace_back(uv);
			}
		}

		void read_face_data(BinaryReader &reader,
		                    const Header &header,
		                    const Element &element,
		                    TriangleMeshData &data)
		{
			std::vector<TriangleMeshData::Index> indices;

			data.indices.reserve(element.count);

			for (Element::Count i = 0; i < element.count; i++)
			{
				for (const Property &property : element.properties)
				{
					if (property.name == Property::Name::VERTEX_INDICES)
					{
						read_list<TriangleMeshData::Index>(reader,
						                                   header,
						                                   property.type,
						                                   indices);
						if (indices.size() < 3)
							throw Exception(reader.position(),
							                "face element must have at least 3 indices");

						TriangleMeshData::Index i1 = indices[0];
						TriangleMeshData::Index i2 = 0;
						TriangleMeshData::Index i3 = indices[1];

						for (std::size_t j = 2; j < indices.size(); j++)
						{
							i2 = i3;
							i3 = indices[j];
							data.indices.emplace_back(i1, i2, i3);
						}
					}
					else
					{
						skip_value(reader, header, property.type);
					}
				}
			}
		}

		TriangleMeshData read_mesh_data(BinaryReader &reader, const Header &header)
		{
			TriangleMeshData data;

			for (const Element &element : header.elements)
			{
				if (element.name == Element::Name::VERTEX)
				{
					read_vertex_data(reader, header, element, data);
				}
				else if (element.name == Element::Name::FACE)
				{
					read_face_data(reader, header, element, data);
				}
				else
				{
					for (Element::Count i = 0; i < element.count; i++)
						for (const Property &property : element.properties)
							skip_value(reader, header, property.type);
				}
			}

			if (data.points.size() < 3)
				throw Exception(reader.position(), "number of vertices must be at least 3");

			if (data.indices.empty())
				throw Exception(reader.position(), "missing indices");

			auto max_index = data.points.size() - 1;

			for (auto &indices : data.indices)
				if (indices.index1 > max_index || indices.index2 > max_index ||
				    indices.index3 > max_index)
					throw Exception(reader.position(),
					                "invalid indices (" + std::to_string(indices.index1) + ", " +
					                        std::to_string(indices.index2) + ", " +
					                        std::to_string(indices.index3) + ")" +
					                        ", max allowed: " + std::to_string(max_index));

			return data;
		}

		TriangleMeshData read_ply(BinaryReader &reader)
		{
			Header header = read_header(reader);

			return read_mesh_data(reader, header);
		}
	}

	Result<TriangleMeshData> ply_read_file(const std::string &file_name)
	{
		TriangleMeshData mesh_data;

		try
		{
			BinaryReader reader;
			reader.open_file(file_name);
			mesh_data = read_ply(reader);
		}
		catch (const BinaryReader::Exception &e)
		{
			return Error(e.what());
		}

		return mesh_data;
	}

	Result<TriangleMeshData> ply_read_data(std::vector<std::uint8_t> &&data)
	{
		TriangleMeshData mesh_data;

		try
		{
			BinaryReader reader;
			reader.set_data(std::move(data));
			mesh_data = read_ply(reader);
		}
		catch (const BinaryReader::Exception &e)
		{
			return Error(e.what());
		}

		return mesh_data;
	}
}
