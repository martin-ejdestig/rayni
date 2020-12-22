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

#ifndef RAYNI_LIB_CONTAINERS_VARIANT_H
#define RAYNI_LIB_CONTAINERS_VARIANT_H

#include <map>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Rayni
{
	class Variant
	{
	public:
		class Exception;

		using Map = std::map<std::string, Variant>;
		using Vector = std::vector<Variant>;

		Variant() = default;

		Variant(const Variant &) = delete;

		Variant(Variant &&other) noexcept
		{
			initialize_from(std::move(other));
		}

		explicit Variant(Map &&map) noexcept : type_(Type::MAP)
		{
			new (&value_.map) Map(std::move(map));
			reparent_children();
		}

		// TODO: Variant::map() should be replaced with a
		//       Variant(std::initializer_list<Map::value_type>). Not possible right
		//       now since std::initializer_list requires the type to be copyable.
		//       Will be fixed in C++20 with N4166? See
		//       http://open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0065r0.pdf .
		static Variant map()
		{
			return Variant(Map());
		}

		template <typename... Args>
		static Variant map(Args &&...args)
		{
			Map map;
			fill_map(map, std::forward<Args>(args)...);
			return Variant(std::move(map));
		}

		explicit Variant(Vector &&vector) noexcept : type_(Type::VECTOR)
		{
			new (&value_.vector) Vector(std::move(vector));
			reparent_children();
		}

		// TODO: Variant::vector() should be replaced with a
		//       Variant(std::initializer_list<Variant>). See TODO for Variant::map() above.
		static Variant vector()
		{
			return Variant(Vector());
		}

		template <typename... Args>
		static Variant vector(Args &&...args)
		{
			Vector vector;
			fill_vector(vector, std::forward<Args>(args)...);
			return Variant(std::move(vector));
		}

		explicit Variant(bool boolean) noexcept : type_(Type::BOOL)
		{
			value_.boolean = boolean;
		}

		explicit Variant(int number) noexcept : type_(Type::INT)
		{
			value_.number_int = number;
		}

		explicit Variant(unsigned int number) noexcept : type_(Type::UNSIGNED_INT)
		{
			value_.number_unsigned_int = number;
		}

		explicit Variant(float number) noexcept : type_(Type::FLOAT)
		{
			value_.number_float = number;
		}

		explicit Variant(double number) noexcept : type_(Type::DOUBLE)
		{
			value_.number_double = number;
		}

		explicit Variant(const char *string) : Variant(std::string(string))
		{
		}

		explicit Variant(std::string &&string) noexcept : type_(Type::STRING)
		{
			new (&value_.string) std::string(std::move(string));
		}

		explicit Variant(const std::string &string) : type_(Type::STRING)
		{
			new (&value_.string) std::string(string);
		}

		~Variant()
		{
			reset_to_none();
		}

		Variant &operator=(const Variant &) = delete;

		Variant &operator=(Variant &&other) noexcept
		{
			reset_to_none();
			initialize_from(std::move(other));
			return *this;
		}

		bool is_none() const
		{
			return type_ == Type::NONE;
		}

		bool is_map() const
		{
			return type_ == Type::MAP;
		}

		bool is_vector() const
		{
			return type_ == Type::VECTOR;
		}

		bool is_bool() const
		{
			return type_ == Type::BOOL;
		}

		bool is_int() const
		{
			return type_ == Type::INT;
		}

		bool is_unsigned_int() const
		{
			return type_ == Type::UNSIGNED_INT;
		}

		bool is_float() const
		{
			return type_ == Type::FLOAT;
		}

		bool is_double() const
		{
			return type_ == Type::DOUBLE;
		}

		bool is_string() const
		{
			return type_ == Type::STRING;
		}

		Map &as_map()
		{
			require_type(Type::MAP);
			return value_.map;
		}

		const Map &as_map() const
		{
			require_type(Type::MAP);
			return value_.map;
		}

		Vector &as_vector()
		{
			require_type(Type::VECTOR);
			return value_.vector;
		}

		const Vector &as_vector() const
		{
			require_type(Type::VECTOR);
			return value_.vector;
		}

		const bool &as_bool() const
		{
			require_type(Type::BOOL);
			return value_.boolean;
		}

		const int &as_int() const
		{
			require_type(Type::INT);
			return value_.number_int;
		}

		const unsigned int &as_unsigned_int() const
		{
			require_type(Type::UNSIGNED_INT);
			return value_.number_unsigned_int;
		}

		const float &as_float() const
		{
			require_type(Type::FLOAT);
			return value_.number_float;
		}

		const double &as_double() const
		{
			require_type(Type::DOUBLE);
			return value_.number_double;
		}

		const std::string &as_string() const
		{
			require_type(Type::STRING);
			return value_.string;
		}

		bool has(const std::string &key) const
		{
			auto i = map_iterator(key);
			return i != value_.map.cend();
		}

		const Variant &get(const std::string &key) const;

		template <typename T>
		T get(const std::string &key) const
		{
			return get(key).to<T>();
		}

		template <typename T>
		T get(const std::string &key, const T &default_value) const
		{
			auto i = map_iterator(key);
			return i == value_.map.cend() ? default_value : i->second.to<T>();
		}

		const Variant &get(std::size_t index) const;

		template <typename T>
		T get(std::size_t index) const
		{
			return get(index).to<T>();
		}

		bool to_bool() const;
		int to_int() const;
		unsigned int to_unsigned_int() const;
		float to_float() const;
		double to_double() const;
		std::string to_string() const;

		template <typename T>
		auto to() const;

		std::string path() const;

	private:
		enum class Type
		{
			NONE,

			MAP,
			VECTOR,

			BOOL,

			INT,
			UNSIGNED_INT,
			FLOAT,
			DOUBLE,

			STRING
		};

		// TODO: fill_map() should be removed when std::initializer_list can handle
		//       non-copyable types. See TODO for Variant::map() above.
		static void fill_map(Map &map, const std::string &key, Variant &&value)
		{
			map.emplace(key, std::move(value));
		}

		template <typename T>
		static void fill_map(Map &map, const std::string &key, const T &value)
		{
			fill_map(map, key, Variant(value));
		}

		template <typename... Args>
		static void fill_map(Map &map, const std::string &key, Variant &&value, Args &&...args)
		{
			map.emplace(key, std::move(value));
			fill_map(map, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static void fill_map(Map &map, const std::string &key, const T &value, Args &&...args)
		{
			return fill_map(map, key, Variant(value), std::forward<Args>(args)...);
		}

		// TODO: fill_vector() should be removed when std::initializer_list can handle
		//       non-copyable types. See TODO for Variant::vector() above.
		static void fill_vector(Vector &vector, Variant &&value)
		{
			vector.emplace_back(std::move(value));
		}

		template <typename T>
		static void fill_vector(Vector &vector, const T &value)
		{
			return fill_vector(vector, Variant(value));
		}

		template <typename... Args>
		static void fill_vector(Vector &vector, Variant &&value, Args &&...args)
		{
			vector.emplace_back(std::move(value));
			fill_vector(vector, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static void fill_vector(Vector &vector, const T &value, Args &&...args)
		{
			return fill_vector(vector, Variant(value), std::forward<Args>(args)...);
		}

		void reset_to_none() noexcept;
		void initialize_from(Variant &&other) noexcept;
		void reparent_children() noexcept;

		Map::const_iterator map_iterator(const std::string &key) const;

		std::string key_in_parent() const;
		std::size_t index_in_parent() const;

		std::string prepend_path_if_has_parent(const std::string &str) const;

		void require_type(Type required_type) const;

		static std::string map_to_string(const Map &map);
		static std::string vector_to_string(const Vector &vector);

		std::string type_to_string() const
		{
			return type_to_string(type_);
		}
		static std::string type_to_string(Type type);

		const Variant *parent_ = nullptr;
		Type type_ = Type::NONE;

		union Value
		{
			Value() // NOLINT(modernize-use-equals-default)
			{
			}

			Value(Value &other) = delete;
			Value(Value &&other) = delete;

			~Value() // NOLINT(modernize-use-equals-default)
			{
			}

			Value &operator=(const Value &other) = delete;
			Value &operator=(Value &&other) = delete;

			Map map;
			Vector vector;

			bool boolean;

			int number_int;
			unsigned int number_unsigned_int;
			float number_float;
			double number_double;

			std::string string;
		} value_;
	};

	class Variant::Exception : public std::runtime_error
	{
	public:
		Exception(const Variant &variant, const std::string &str) :
		        std::runtime_error(variant.prepend_path_if_has_parent(str))
		{
		}
	};

	template <typename T>
	auto Variant::to() const
	{
		if constexpr (std::is_same_v<T, bool>)
			return to_bool();
		else if constexpr (std::is_same_v<T, int>)
			return to_int();
		else if constexpr (std::is_same_v<T, unsigned int>)
			return to_unsigned_int();
		else if constexpr (std::is_same_v<T, float>)
			return to_float();
		else if constexpr (std::is_same_v<T, double>)
			return to_double();
		else if constexpr (std::is_same_v<T, std::string>)
			return to_string();
		else if constexpr (std::is_constructible_v<T, const Variant &>)
			return T(*this);
		else if constexpr (std::is_pointer_v<T>)
			return std::remove_pointer_t<T>::get_from_variant(*this);
		else
			return T::from_variant(*this);
	}
}

#endif // RAYNI_LIB_CONTAINERS_VARIANT_H
