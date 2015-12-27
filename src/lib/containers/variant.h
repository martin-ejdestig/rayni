/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015 Martin Ejdestig <marejde@gmail.com>
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

#ifndef _RAYNI_LIB_CONTAINERS_VARIANT_H_
#define _RAYNI_LIB_CONTAINERS_VARIANT_H_

#include <map>
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
		class Exception : public std::runtime_error
		{
		public:
			Exception(const Variant &variant, const std::string &str)
			        : std::runtime_error(variant.prepend_path_if_has_parent(str))
			{
			}
		};

		using Map = std::map<std::string, Variant>;
		using Vector = std::vector<Variant>;

		Variant()
		{
		}

		Variant(const Variant &) = delete;

		Variant(Variant &&other) noexcept
		{
			initialize_from(std::move(other));
		}

		explicit Variant(Map &&map) : type(Type::MAP)
		{
			new (&value.map) Map(std::move(map));
			reparent_children();
		}

		explicit Variant(Vector &&vector) : type(Type::VECTOR)
		{
			new (&value.vector) Vector(std::move(vector));
			reparent_children();
		}

		explicit Variant(bool boolean) : type(Type::BOOL)
		{
			value.boolean = boolean;
		}

		explicit Variant(int number) : type(Type::INT)
		{
			value.number_int = number;
		}

		explicit Variant(unsigned int number) : type(Type::UNSIGNED_INT)
		{
			value.number_unsigned_int = number;
		}

		explicit Variant(float number) : type(Type::FLOAT)
		{
			value.number_float = number;
		}

		explicit Variant(double number) : type(Type::DOUBLE)
		{
			value.number_double = number;
		}

		explicit Variant(const char *string) : Variant(std::string(string))
		{
		}

		explicit Variant(std::string &&string) : type(Type::STRING)
		{
			new (&value.string) std::string(std::move(string));
		}

		explicit Variant(const std::string &string) : type(Type::STRING)
		{
			new (&value.string) std::string(string);
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
			return type == Type::NONE;
		}

		bool is_map() const
		{
			return type == Type::MAP;
		}

		bool is_vector() const
		{
			return type == Type::VECTOR;
		}

		bool is_bool() const
		{
			return type == Type::BOOL;
		}

		bool is_int() const
		{
			return type == Type::INT;
		}

		bool is_unsigned_int() const
		{
			return type == Type::UNSIGNED_INT;
		}

		bool is_float() const
		{
			return type == Type::FLOAT;
		}

		bool is_double() const
		{
			return type == Type::DOUBLE;
		}

		bool is_string() const
		{
			return type == Type::STRING;
		}

		template <typename T>
		bool is() const;

		const Map &get_map() const
		{
			require_type(Type::MAP);
			return value.map;
		}

		const Vector &get_vector() const
		{
			require_type(Type::VECTOR);
			return value.vector;
		}

		const bool &get_bool() const
		{
			require_type(Type::BOOL);
			return value.boolean;
		}

		const int &get_int() const
		{
			require_type(Type::INT);
			return value.number_int;
		}

		const unsigned int &get_unsigned_int() const
		{
			require_type(Type::UNSIGNED_INT);
			return value.number_unsigned_int;
		}

		const float &get_float() const
		{
			require_type(Type::FLOAT);
			return value.number_float;
		}

		const double &get_double() const
		{
			require_type(Type::DOUBLE);
			return value.number_double;
		}

		const std::string &get_string() const
		{
			require_type(Type::STRING);
			return value.string;
		}

		template <typename T>
		const T &get_value() const;

		const Variant &get(const std::string &key) const;

		template <typename T>
		T get(const std::string &key) const
		{
			return get(key).to<T>();
		}

		template <typename T>
		T get(const std::string &key, const T &default_value) const
		{
			auto i = get_map_iterator(key);
			return i == value.map.cend() ? default_value : i->second.to<T>();
		}

		const Variant &get(std::size_t index) const;

		template <typename T>
		T get(std::size_t index) const
		{
			return get(index).to<T>();
		}

		int to_int() const;
		unsigned int to_unsigned_int() const;
		float to_float() const;
		double to_double() const;
		std::string to_string() const;

		template <typename T>
		std::enable_if_t<std::is_same<T, int>::value, T> to() const
		{
			return to_int();
		}

		template <typename T>
		std::enable_if_t<std::is_same<T, unsigned int>::value, T> to() const
		{
			return to_unsigned_int();
		}

		template <typename T>
		std::enable_if_t<std::is_same<T, float>::value, T> to() const
		{
			return to_float();
		}

		template <typename T>
		std::enable_if_t<std::is_same<T, double>::value, T> to() const
		{
			return to_double();
		}

		template <typename T>
		std::enable_if_t<std::is_same<T, std::string>::value, T> to() const
		{
			return to_string();
		}

		template <typename T>
		std::enable_if_t<std::is_constructible<T, const Variant &>::value, T> to() const
		{
			return T(*this);
		}

		template <typename T>
		std::enable_if_t<!std::is_constructible<T, const Variant &>::value && std::is_class<T>::value &&
		                         !std::is_same<T, std::string>::value,
		                 T>
		to() const
		{
			return T::from_variant(*this);
		}

		template <typename T>
		std::enable_if_t<std::is_pointer<T>::value, T> to() const
		{
			// Ownership is NOT transfered!
			return std::remove_pointer<T>::type::get_from_variant(*this);
		}

		std::string get_path() const;

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

		void reset_to_none() noexcept;
		void initialize_from(Variant &&other) noexcept;
		void reparent_children();

		Map::const_iterator get_map_iterator(const std::string &key) const;

		std::string prepend_path_if_has_parent(const std::string &str) const;

		void require_type(Type required_type) const;

		static std::string map_to_string(const Map &map);
		static std::string vector_to_string(const Vector &vector);

		std::string type_to_string() const
		{
			return type_to_string(type);
		}
		static std::string type_to_string(Type type);

		const Variant *parent = nullptr;
		Type type = Type::NONE;

		union Value
		{
			Value()
			{
			}

			~Value()
			{
			}

			Map map;
			Vector vector;

			bool boolean;

			int number_int;
			unsigned int number_unsigned_int;
			float number_float;
			double number_double;

			std::string string;
		} value;
	};

	template <>
	inline bool Variant::is<Variant::Map>() const
	{
		return is_map();
	}

	template <>
	inline bool Variant::is<Variant::Vector>() const
	{
		return is_vector();
	}

	template <>
	inline bool Variant::is<bool>() const
	{
		return is_bool();
	}

	template <>
	inline bool Variant::is<int>() const
	{
		return is_int();
	}

	template <>
	inline bool Variant::is<unsigned int>() const
	{
		return is_unsigned_int();
	}

	template <>
	inline bool Variant::is<float>() const
	{
		return is_float();
	}

	template <>
	inline bool Variant::is<double>() const
	{
		return is_double();
	}

	template <>
	inline bool Variant::is<std::string>() const
	{
		return is_string();
	}

	template <>
	inline const Variant::Map &Variant::get_value() const
	{
		return get_map();
	}

	template <>
	inline const Variant::Vector &Variant::get_value() const
	{
		return get_vector();
	}

	template <>
	inline const bool &Variant::get_value() const
	{
		return get_bool();
	}

	template <>
	inline const int &Variant::get_value() const
	{
		return get_int();
	}

	template <>
	inline const unsigned int &Variant::get_value() const
	{
		return get_unsigned_int();
	}

	template <>
	inline const float &Variant::get_value() const
	{
		return get_float();
	}

	template <>
	inline const double &Variant::get_value() const
	{
		return get_double();
	}

	template <>
	inline const std::string &Variant::get_value() const
	{
		return get_string();
	}
}

#endif // _RAYNI_LIB_CONTAINERS_VARIANT_H_
