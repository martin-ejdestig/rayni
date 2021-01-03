// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include <array>
#include <cassert>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "lib/function/result.h"

namespace Rayni
{
	// Simple variant class. Mainly used instead of std::variant since:
	// - It was created long before std::variant existed.
	// - std::variant lacks some features in Variant (e.g. to() and path()).
	// - The std::variant API is not good.
	// std::variant was used internally in Variant instead of a union for a while but
	// reverted it due to it generating worse code (at least when using libstdc++/libc++ and
	// GCC/Clang). E.g. a lot of unnecessary exception code for accessing wrong value type even
	// though type check was done just prior to getting value etc.
	class Variant
	{
	public:
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
			assert(is_map());
			return value_.map;
		}

		const Map &as_map() const
		{
			assert(is_map());
			return value_.map;
		}

		Vector &as_vector()
		{
			assert(is_vector());
			return value_.vector;
		}

		const Vector &as_vector() const
		{
			assert(is_vector());
			return value_.vector;
		}

		const bool &as_bool() const
		{
			assert(is_bool());
			return value_.boolean;
		}

		const int &as_int() const
		{
			assert(is_int());
			return value_.number_int;
		}

		const unsigned int &as_unsigned_int() const
		{
			assert(is_unsigned_int());
			return value_.number_unsigned_int;
		}

		const float &as_float() const
		{
			assert(is_float());
			return value_.number_float;
		}

		const double &as_double() const
		{
			assert(is_double());
			return value_.number_double;
		}

		const std::string &as_string() const
		{
			assert(is_string());
			return value_.string;
		}

		bool has(const std::string &key) const
		{
			if (!is_map())
				return false;
			return value_.map.find(key) != value_.map.cend();
		}

		const Variant *get(const std::string &key) const
		{
			if (!is_map())
				return nullptr;
			auto i = value_.map.find(key);
			return i != value_.map.cend() ? &i->second : nullptr;
		}

		template <typename T>
		Result<T> get(const std::string &key) const
		{
			if (const Variant *v = get(key); v)
				return v->to<T>();
			return Error(path(), "key \"" + key + "\" not found");
		}

		template <typename T>
		Result<T> get(const std::string &key, const T &default_value) const
		{
			if (const Variant *v = get(key); v)
				return v->to<T>();
			return T(default_value);
		}

		template <typename T>
		Result<T> get(const std::string &key, T &&default_value) const
		{
			if (const Variant *v = get(key); v)
				return v->to<T>();
			return std::forward<T>(default_value);
		}

		const Variant *get(std::size_t index) const
		{
			if (!is_vector())
				return nullptr;
			if (index >= value_.vector.size())
				return nullptr;
			return &value_.vector[index];
		}

		template <typename T>
		Result<T> get(std::size_t index) const
		{
			if (const Variant *v = get(index); v)
				return v->to<T>();
			return Error(path(), "index " + std::to_string(index) + " does not exist");
		}

		Result<bool> to_bool() const;
		Result<int> to_int() const;
		Result<unsigned int> to_unsigned_int() const;
		Result<float> to_float() const;
		Result<double> to_double() const;
		Result<std::string> to_string() const;

		template <typename T, std::size_t N>
		Result<std::array<T, N>> to_array() const
		{
			if (!is_vector() || as_vector().size() != N)
				return Error(path(), "cannot convert to array of size " + std::to_string(N));

			std::array<T, N> values;

			for (std::size_t i = 0; i < N; i++)
				if (auto r = value_.vector[i].to<T>(); !r)
					return r.error();
				else
					values[i] = *r;

			return values;
		}

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

		std::string key_in_parent() const;
		std::size_t index_in_parent() const;

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
		else if constexpr (std::is_pointer_v<T>)
			return std::remove_pointer_t<T>::get_from_variant(*this);
		else
			return T::from_variant(*this);
	}
}

#endif // RAYNI_LIB_CONTAINERS_VARIANT_H
