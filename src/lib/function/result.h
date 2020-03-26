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

#ifndef RAYNI_LIB_FUNCTION_RESULT_H
#define RAYNI_LIB_FUNCTION_RESULT_H

#include <cassert>
#include <string>
#include <system_error>
#include <utility>
#include <variant>

// TODO: clang-format can not handle [[nodiscard]]. Remove clang-format off/on once
//       https://bugs.llvm.org/show_bug.cgi?id=38401 is fixed. Until then, temporarily remove
//       [[nodiscard]], format and add it back again when changing this file.
// clang-format off

namespace Rayni
{
	// Result allows for returning a value (or void) or an error. The error
	// contains a message string and an optional std::error_code.
	//
	// TODO: What is the status of std::expected? It did not make it into
	//       C++20. Check in > C++20 if something useful has been added to
	//       the standard that can replace Result or be used in it.

	class Error
	{
	public:
		explicit Error(std::string &&message) : message_(std::move(message))
		{
		}

		Error(const std::string &message, std::error_code error_code) :
		        message_(message),
		        error_code_(error_code)
		{
		}

		std::string message() const
		{
			std::string ret = message_;

			if (error_code_)
				ret += ": " + error_code_.message();

			return ret;
		}

	private:
		std::string message_;
		std::error_code error_code_;
	};

	template <typename T>
	class ResultBase
	{
	public:
		explicit operator bool() const
		{
			return !is_error();
		}

		bool is_error() const
		{
			return std::holds_alternative<Error>(value_or_error_);
		}

		const Error &error() const
		{
			assert(is_error());
			return std::get<Error>(value_or_error_);
		}

	protected:
		explicit ResultBase(T &&value) : value_or_error_(std::move(value))
		{
		}

		explicit ResultBase(Error &&error) : value_or_error_(std::move(error))
		{
		}

		explicit ResultBase(const Error &error) : value_or_error_(error)
		{
		}

		std::variant<T, Error> &value_or_error()
		{
			return value_or_error_;
		}

		const std::variant<T, Error> &value_or_error() const
		{
			return value_or_error_;
		}

	private:
		std::variant<T, Error> value_or_error_;
	};

	template <typename T>
	class [[nodiscard]] Result final : public ResultBase<T>
	{
	public:
		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(T &&value) : ResultBase<T>(std::move(value))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) : ResultBase<T>(std::move(error))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) : ResultBase<T>(error)
		{
		}

		T &operator*() &
		{
			return value();
		}

		const T &operator*() const &
		{
			return value();
		}

		T &&operator*() &&
		{
			return std::move(value());
		}

		const T &&operator*() const &&
		{
			return std::move(value());
		}

		T *operator->()
		{
			return &value();
		}

		const T *operator->() const
		{
			return &value();
		}

		T &value() &
		{
			assert(!this->is_error());
			return std::get<T>(this->value_or_error());
		}

		const T &value() const &
		{
			assert(!this->is_error());
			return std::get<T>(this->value_or_error());
		}

		T &&value() &&
		{
			assert(!this->is_error());
			return std::move(std::get<T>(this->value_or_error()));
		}

		const T &&value() const &&
		{
			assert(!this->is_error());
			return std::move(std::get<T>(this->value_or_error()));
		}

		T value_or(T &&default_value) const &
		{
			if (this->is_error())
				return std::forward<T>(default_value);

			return value();
		}

		T value_or(T &&default_value) &&
		{
			if (this->is_error())
				return std::forward<T>(default_value);

			return std::move(value());
		}
	};

	template <>
	class [[nodiscard]] Result<void> final : public ResultBase<std::monostate>
	{
	public:
		Result() : ResultBase<std::monostate>(std::monostate())
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) : ResultBase<std::monostate>(std::move(error))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) : ResultBase<std::monostate>(error)
		{
		}
	};
}

// TODO: Remove once clang-format bug is fixed. See above.
// clang-format on

#endif // RAYNI_LIB_FUNCTION_RESULT_H
