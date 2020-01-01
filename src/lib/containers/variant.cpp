// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/containers/variant.h"

#include <cassert>
#include <string>

#include "lib/math/numeric_cast.h"

namespace Rayni
{
	Variant::Variant(Variant &&other) noexcept : value_(std::move(other.value_))
	{
		other.value_ = std::monostate();
		reparent_children();
	}

	Variant &Variant::operator=(Variant &&other) noexcept
	{
		value_ = std::move(other.value_);
		other.value_ = std::monostate();
		reparent_children();
		return *this;
	}

	void Variant::reparent_children() noexcept
	{
		if (is_map())
		{
			for (auto &[key, value] : as_map())
				value.parent_ = this;
		}
		else if (is_vector())
		{
			for (auto &v : as_vector())
				v.parent_ = this;
		}
	}

	Variant::Map &Variant::as_map()
	{
		if (auto v = std::get_if<Map>(&value_); v)
			return *v;
		throw Exception(*this, "expected map");
	}

	const Variant::Map &Variant::as_map() const
	{
		if (auto v = std::get_if<Map>(&value_); v)
			return *v;
		throw Exception(*this, "expected map");
	}

	Variant::Vector &Variant::as_vector()
	{
		if (auto v = std::get_if<Vector>(&value_); v)
			return *v;
		throw Exception(*this, "expected vector");
	}

	const Variant::Vector &Variant::as_vector() const
	{
		if (auto v = std::get_if<Vector>(&value_); v)
			return *v;
		throw Exception(*this, "expected vector");
	}

	const bool &Variant::as_bool() const
	{
		if (auto v = std::get_if<bool>(&value_); v)
			return *v;
		throw Exception(*this, "expected bool");
	}

	const int &Variant::as_int() const
	{
		if (auto v = std::get_if<int>(&value_); v)
			return *v;
		throw Exception(*this, "expected int");
	}

	const unsigned int &Variant::as_unsigned_int() const
	{
		if (auto v = std::get_if<unsigned int>(&value_); v)
			return *v;
		throw Exception(*this, "expected unsigned int");
	}

	const float &Variant::as_float() const
	{
		if (auto v = std::get_if<float>(&value_); v)
			return *v;
		throw Exception(*this, "expected float");
	}

	const double &Variant::as_double() const
	{
		if (auto v = std::get_if<double>(&value_); v)
			return *v;
		throw Exception(*this, "expected double");
	}

	const std::string &Variant::as_string() const
	{
		if (auto v = std::get_if<std::string>(&value_); v)
			return *v;
		throw Exception(*this, "expected string");
	}

	bool Variant::has(const std::string &key) const
	{
		const Map &map = as_map();
		auto i = map.find(key);
		return i != map.cend();
	}

	const Variant &Variant::get(const std::string &key) const
	{
		const Map &map = as_map();
		auto i = map.find(key);
		if (i == map.cend())
			throw Exception(*this, "key \"" + key + "\" not found");
		return i->second;
	}

	const Variant &Variant::get(std::size_t index) const
	{
		const Vector &vector = as_vector();
		if (index >= vector.size())
			throw Exception(*this, "index \"" + std::to_string(index) + "\" out of bounds");
		return vector[index];
	}

	bool Variant::to_bool() const
	{
		if (is_bool())
			return as_bool();

		throw Exception(*this, "cannot convert to bool");
	}

	int Variant::to_int() const
	{
		if (is_int())
			return as_int();

		std::optional<int> result;

		if (is_unsigned_int())
			result = numeric_cast<int>(as_unsigned_int());
		else if (is_float())
			result = numeric_cast<int>(as_float());
		else if (is_double())
			result = numeric_cast<int>(as_double());

		if (!result.has_value())
			throw Exception(*this, "cannot convert to int");

		return result.value();
	}

	unsigned int Variant::to_unsigned_int() const
	{
		if (is_unsigned_int())
			return as_unsigned_int();

		std::optional<unsigned int> result;

		if (is_int())
			result = numeric_cast<unsigned int>(as_int());
		else if (is_float())
			result = numeric_cast<unsigned int>(as_float());
		else if (is_double())
			result = numeric_cast<unsigned int>(as_double());

		if (!result.has_value())
			throw Exception(*this, "cannot convert to unsigned int");

		return result.value();
	}

	float Variant::to_float() const
	{
		if (is_float())
			return as_float();

		std::optional<float> result;

		if (is_int())
			result = numeric_cast<float>(as_int());
		else if (is_unsigned_int())
			result = numeric_cast<float>(as_unsigned_int());
		else if (is_double())
			result = numeric_cast<float>(as_double());

		if (!result.has_value())
			throw Exception(*this, "cannot convert to float");

		return result.value();
	}

	double Variant::to_double() const
	{
		if (is_double())
			return as_double();

		std::optional<double> result;

		if (is_int())
			result = numeric_cast<double>(as_int());
		else if (is_unsigned_int())
			result = numeric_cast<double>(as_unsigned_int());
		else if (is_float())
			result = numeric_cast<double>(as_float());

		if (!result.has_value())
			throw Exception(*this, "cannot convert to double");

		return result.value();
	}

	std::string Variant::to_string() const
	{
		if (is_map())
			return map_to_string(as_map());
		if (is_vector())
			return vector_to_string(as_vector());
		if (is_bool())
			return as_bool() ? "true" : "false";
		if (is_int())
			return std::to_string(as_int());
		if (is_unsigned_int())
			return std::to_string(as_unsigned_int());
		if (is_float())
			return std::to_string(as_float());
		if (is_double())
			return std::to_string(as_double());
		if (is_string())
			return as_string();

		throw Exception(*this, "cannot convert to string");
	}

	std::string Variant::map_to_string(const Map &map)
	{
		std::string delimiter;
		std::string str = "{ ";

		for (auto &[key, value] : map)
		{
			if (delimiter.empty())
				delimiter = ", ";
			else
				str += delimiter;

			str += key + ": " + value.to_string();
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
		if (!parent_)
			return "";

		const std::string parent_path = parent_->path();

		if (parent_->is_map())
			return parent_path + "['" + key_in_parent() + "']";

		if (parent_->is_vector())
			return parent_path + "[" + std::to_string(index_in_parent()) + "]";

		return parent_path;
	}

	std::string Variant::key_in_parent() const
	{
		assert(parent_ && parent_->is_map());

		for (auto &[key, value] : parent_->as_map())
			if (&value == this)
				return key;

		assert(false);
		return "";
	}

	std::size_t Variant::index_in_parent() const
	{
		assert(parent_ && parent_->is_vector());

		return static_cast<std::size_t>(this - &parent_->as_vector()[0]);
	}

	std::string Variant::prepend_path_if_has_parent(const std::string &str) const
	{
		return parent_ ? path() + ": " + str : str;
	}
}
