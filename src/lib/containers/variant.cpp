/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2016 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/containers/variant.h"

#include <cassert>
#include <cmath>
#include <string>

namespace Rayni
{
	void Variant::reset_to_none() noexcept
	{
		switch (type)
		{
		case Type::NONE:
			break;
		case Type::MAP:
			value.map.~Map();
			break;
		case Type::VECTOR:
			value.vector.~Vector();
			break;
		case Type::BOOL:
			value.boolean = false;
			break;
		case Type::INT:
			value.number_int = 0;
			break;
		case Type::UNSIGNED_INT:
			value.number_unsigned_int = 0;
			break;
		case Type::FLOAT:
			value.number_float = 0;
			break;
		case Type::DOUBLE:
			value.number_double = 0;
			break;
		case Type::STRING:
			// TODO: This should be:
			// value.string.std::string::~string();
			// But want to be able to test compile with Clang as well as GCC and there
			// is a bug in Clang. See http://llvm.org/bugs/show_bug.cgi?id=12350 and its
			// dependencies. Temporary workaround with using until Clang has been fixed.
			using String = std::string;
			value.string.~String();
			break;
		}

		type = Type::NONE;
	}

	void Variant::initialize_from(Variant &&other) noexcept
	{
		assert(type == Type::NONE);

		switch (other.type)
		{
		case Type::NONE:
			break;
		case Type::MAP:
			new (&value.map) Map(std::move(other.value.map));
			break;
		case Type::VECTOR:
			new (&value.vector) Vector(std::move(other.value.vector));
			break;
		case Type::BOOL:
			value.boolean = other.value.boolean;
			break;
		case Type::INT:
			value.number_int = other.value.number_int;
			break;
		case Type::UNSIGNED_INT:
			value.number_unsigned_int = other.value.number_unsigned_int;
			break;
		case Type::FLOAT:
			value.number_float = other.value.number_float;
			break;
		case Type::DOUBLE:
			value.number_double = other.value.number_double;
			break;
		case Type::STRING:
			new (&value.string) std::string(std::move(other.value.string));
			break;
		}

		type = other.type;
		reparent_children();

		other.reset_to_none();
	}

	void Variant::reparent_children()
	{
		if (is_map())
		{
			for (auto &key_value : value.map)
				key_value.second.parent = this;
		}
		else if (is_vector())
		{
			for (auto &v : value.vector)
				v.parent = this;
		}
	}

	const Variant &Variant::get(const std::string &key) const
	{
		auto i = map_iterator(key);
		if (i == value.map.cend())
			throw Exception(*this, "key \"" + key + "\" not found");
		return i->second;
	}

	Variant::Map::const_iterator Variant::map_iterator(const std::string &key) const
	{
		require_type(Type::MAP);
		return value.map.find(key);
	}

	const Variant &Variant::get(std::size_t index) const
	{
		require_type(Type::VECTOR);
		if (index >= value.vector.size())
			throw Exception(*this, "index \"" + std::to_string(index) + "\" out of bounds");
		return value.vector[index];
	}

	int Variant::to_int() const
	{
		if (is_int())
			return value.number_int;
		if (is_unsigned_int())
			return static_cast<int>(value.number_unsigned_int);
		if (is_float())
			return static_cast<int>(std::round(value.number_float));
		if (is_double())
			return static_cast<int>(std::round(value.number_double));

		throw Exception(*this, "cannot convert \"" + type_to_string() + "\" to \"int\"");
	}

	unsigned int Variant::to_unsigned_int() const
	{
		if (is_int())
			return static_cast<unsigned int>(value.number_int);
		if (is_unsigned_int())
			return value.number_unsigned_int;
		if (is_float())
			return static_cast<unsigned int>(std::round(value.number_float));
		if (is_double())
			return static_cast<unsigned int>(std::round(value.number_double));

		throw Exception(*this, "cannot convert \"" + type_to_string() + "\" to \"unsigned int\"");
	}

	float Variant::to_float() const
	{
		if (is_int())
			return static_cast<float>(value.number_int);
		if (is_unsigned_int())
			return static_cast<float>(value.number_unsigned_int);
		if (is_float())
			return value.number_float;
		if (is_double())
			return static_cast<float>(value.number_double);

		throw Exception(*this, "cannot convert \"" + type_to_string() + "\" to \"float\"");
	}

	double Variant::to_double() const
	{
		if (is_int())
			return static_cast<double>(value.number_int);
		if (is_unsigned_int())
			return static_cast<double>(value.number_unsigned_int);
		if (is_float())
			return static_cast<double>(value.number_float);
		if (is_double())
			return value.number_double;

		throw Exception(*this, "cannot convert \"" + type_to_string() + "\" to \"double\"");
	}

	std::string Variant::to_string() const
	{
		switch (type)
		{
		case Type::NONE:
			break;
		case Type::MAP:
			return map_to_string(value.map);
		case Type::VECTOR:
			return vector_to_string(value.vector);
		case Type::BOOL:
			return value.boolean ? "true" : "false";
		case Type::INT:
			return std::to_string(value.number_int);
		case Type::UNSIGNED_INT:
			return std::to_string(value.number_unsigned_int);
		case Type::FLOAT:
			return std::to_string(value.number_float);
		case Type::DOUBLE:
			return std::to_string(value.number_double);
		case Type::STRING:
			return value.string;
		}

		throw Exception(*this, "cannot convert \"" + type_to_string() + "\" to \"string\"");
	}

	std::string Variant::map_to_string(const Map &map)
	{
		std::string delimiter;
		std::string str = "{ ";

		for (auto &key_value : map)
		{
			if (delimiter.empty())
				delimiter = ", ";
			else
				str += delimiter;

			str += key_value.first + ": " + key_value.second.to_string();
		}

		str += " }";

		return str;
	}

	std::string Variant::vector_to_string(const Vector &vector)
	{
		std::string delimiter;
		std::string str = "[ ";

		for (auto &v : vector)
		{
			if (delimiter.empty())
				delimiter = ", ";
			else
				str += delimiter;

			str += v.to_string();
		}

		str += " ]";

		return str;
	}

	std::string Variant::path() const
	{
		if (!parent)
			return "";

		std::string parent_path = parent->path();

		if (parent->is_map())
		{
			for (auto &key_value : parent->value.map)
			{
				if (&key_value.second == this)
				{
					const std::string &key = key_value.first;
					return parent_path + "['" + key + "']";
				}
			}
		}
		else if (parent->is_vector())
		{
			auto index = this - &parent->value.vector[0];
			return parent_path + "[" + std::to_string(index) + "]";
		}

		return parent_path;
	}

	std::string Variant::prepend_path_if_has_parent(const std::string &str) const
	{
		return parent ? path() + ": " + str : str;
	}

	void Variant::require_type(Type required_type) const
	{
		if (type != required_type)
			throw Exception(*this,
			                "expected \"" + type_to_string(required_type) + "\" instead of \"" +
			                        type_to_string(type) + "\"");
	}

	std::string Variant::type_to_string(Type type)
	{
		switch (type)
		{
		case Type::NONE:
			break;
		case Type::MAP:
			return "map";
		case Type::VECTOR:
			return "vector";
		case Type::BOOL:
			return "bool";
		case Type::INT:
			return "int";
		case Type::UNSIGNED_INT:
			return "unsigned int";
		case Type::FLOAT:
			return "float";
		case Type::DOUBLE:
			return "double";
		case Type::STRING:
			return "string";
		}

		return "none";
	}
}
