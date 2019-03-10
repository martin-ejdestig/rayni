// This file is part of Rayni.
//
// Copyright (C) 2015-2019 Martin Ejdestig <marejde@gmail.com>
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
#include <variant>
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
		Variant(Variant &&other) noexcept;

		explicit Variant(Map &&map) noexcept : value_(std::move(map))
		{
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
		static Variant map(Args &&... args)
		{
			Map map;
			fill_map(map, std::forward<Args>(args)...);
			return Variant(std::move(map));
		}

		explicit Variant(Vector &&vector) noexcept : value_(std::move(vector))
		{
			reparent_children();
		}

		// TODO: Variant::vector() should be replaced with a
		//       Variant(std::initializer_list<Variant>). See TODO for Variant::map() above.
		static Variant vector()
		{
			return Variant(Vector());
		}

		template <typename... Args>
		static Variant vector(Args &&... args)
		{
			Vector vector;
			fill_vector(vector, std::forward<Args>(args)...);
			return Variant(std::move(vector));
		}

		explicit Variant(bool boolean) noexcept : value_(boolean)
		{
		}

		explicit Variant(int number) noexcept : value_(number)
		{
		}

		explicit Variant(unsigned int number) noexcept : value_(number)
		{
		}

		explicit Variant(float number) noexcept : value_(number)
		{
		}

		explicit Variant(double number) noexcept : value_(number)
		{
		}

		explicit Variant(const char *string) : Variant(std::string(string))
		{
		}

		explicit Variant(std::string &&string) noexcept : value_(std::move(string))
		{
		}

		explicit Variant(const std::string &string) : Variant(std::string(string))
		{
		}

		~Variant() = default;

		Variant &operator=(const Variant &) = delete;
		Variant &operator=(Variant &&other) noexcept;

		bool is_none() const
		{
			return std::holds_alternative<std::monostate>(value_);
		}

		bool is_map() const
		{
			return std::holds_alternative<Map>(value_);
		}

		bool is_vector() const
		{
			return std::holds_alternative<Vector>(value_);
		}

		bool is_bool() const
		{
			return std::holds_alternative<bool>(value_);
		}

		bool is_int() const
		{
			return std::holds_alternative<int>(value_);
		}

		bool is_unsigned_int() const
		{
			return std::holds_alternative<unsigned int>(value_);
		}

		bool is_float() const
		{
			return std::holds_alternative<float>(value_);
		}

		bool is_double() const
		{
			return std::holds_alternative<double>(value_);
		}

		bool is_string() const
		{
			return std::holds_alternative<std::string>(value_);
		}

		Map &as_map();
		const Map &as_map() const;

		Vector &as_vector();
		const Vector &as_vector() const;

		const bool &as_bool() const;
		const int &as_int() const;
		const unsigned int &as_unsigned_int() const;
		const float &as_float() const;
		const double &as_double() const;
		const std::string &as_string() const;

		bool has(const std::string &key) const;

		const Variant &get(const std::string &key) const;

		template <typename T>
		T get(const std::string &key) const
		{
			return get(key).to<T>();
		}

		template <typename T>
		T get(const std::string &key, const T &default_value) const
		{
			const Map &map = as_map();
			auto i = map.find(key);
			return i == map.cend() ? default_value : i->second.to<T>();
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
		static void fill_map(Map &map, const std::string &key, Variant &&value, Args &&... args)
		{
			map.emplace(key, std::move(value));
			fill_map(map, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static void fill_map(Map &map, const std::string &key, const T &value, Args &&... args)
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
		static void fill_vector(Vector &vector, Variant &&value, Args &&... args)
		{
			vector.emplace_back(std::move(value));
			fill_vector(vector, std::forward<Args>(args)...);
		}

		template <typename T, typename... Args>
		static void fill_vector(Vector &vector, const T &value, Args &&... args)
		{
			return fill_vector(vector, Variant(value), std::forward<Args>(args)...);
		}

		void reparent_children() noexcept;

		std::string key_in_parent() const;
		std::size_t index_in_parent() const;

		std::string prepend_path_if_has_parent(const std::string &str) const;

		static std::string map_to_string(const Map &map);
		static std::string vector_to_string(const Vector &vector);

		const Variant *parent_ = nullptr;
		std::variant<std::monostate, Map, Vector, bool, int, unsigned int, float, double, std::string> value_;
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
		// TODO: All statements are not valid for all T so must have "else" after "return".
		//       Can remove the NOLINT(readability-else-after-return) below when
		//       https://bugs.llvm.org/show_bug.cgi?id=32197 is fixed?
		if constexpr (std::is_same_v<T, bool>)
			return to_bool();
		else if constexpr (std::is_same_v<T, int>) // NOLINT(readability-else-after-return)
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
