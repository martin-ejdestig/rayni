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
#include <memory>
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
	// Result allows for returning a value (or void) or an error.
	//
	// Since Result can be used quite a lot an effort has been made to keep the size overhead
	// of Result as small as possible. Goal is to have a maximum size overhead of sizeof(void *).
	// To accomplish this Error is stored as a pointer. This reduces cache locality but only in
	// the case of an Error and most common case by far should be to return success.
	//
	// TODO: Perform more performance measurements to see if reasoning about size is justified.
	//       Code generated for returning error will be larger with separate allocation which in
	//       turn puts higher preassure on instruction cache.
	//
	// TODO: What is the status of std::expected? It did not make it into C++20. Check in >C++20
	//       if something useful has been added to the standard that can replace Result or be
	//       used in it.

	class Error
	{
	public:
		explicit Error(std::string &&message) : message_(std::move(message))
		{
		}

		Error(const std::string &message, std::error_code error_code) :
		        message_(message + ": " + error_code.message())
		{
		}

		const std::string &message() const
		{
			return message_;
		}

	private:
		std::string message_;
	};

	template <typename T>
	class [[nodiscard]] Result final
	{
	public:
		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(T &&value) : value_or_error_(std::move(value))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) : value_or_error_(std::make_unique<Error>(std::move(error)))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) : value_or_error_(std::make_unique<Error>(error))
		{
		}

		explicit operator bool() const
		{
			return !is_error();
		}

		bool is_error() const
		{
			return std::holds_alternative<std::unique_ptr<Error>>(value_or_error_);
		}

		const Error &error() const
		{
			assert(is_error());
			return *std::get<std::unique_ptr<Error>>(value_or_error_);
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
			assert(!is_error());
			return std::get<T>(value_or_error_);
		}

		const T &value() const &
		{
			assert(!is_error());
			return std::get<T>(value_or_error_);
		}

		T &&value() &&
		{
			assert(!is_error());
			return std::move(std::get<T>(value_or_error_));
		}

		const T &&value() const &&
		{
			assert(!is_error());
			return std::move(std::get<T>(value_or_error_));
		}

		T value_or(T &&default_value) const &
		{
			if (is_error())
				return std::forward<T>(default_value);

			return value();
		}

		T value_or(T &&default_value) &&
		{
			if (is_error())
				return std::forward<T>(default_value);

			return std::move(value());
		}

	private:
		// TODO: Consider removing use of std::variant. Generates unnecessary code with exceptions.
		std::variant<T, std::unique_ptr<Error>> value_or_error_;
	};

	template <>
	class [[nodiscard]] Result<void> final
	{
	public:
		Result() = default;

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(Error &&error) : error_(std::make_unique<Error>(std::move(error)))
		{
		}

		// NOLINTNEXTLINE(google-explicit-constructor) Want implicit conversion for less verbosity.
		Result(const Error &error) : error_(std::make_unique<Error>(error))
		{
		}

		explicit operator bool() const
		{
			return !is_error();
		}

		bool is_error() const
		{
			return error_ != nullptr;
		}

		const Error &error() const
		{
			assert(is_error());
			return *error_;
		}

	private:
		std::unique_ptr<Error> error_;
	};

	// See resoning about size at top of this file.
	static_assert(sizeof(Result<void>) <= sizeof(void *));
	static_assert(sizeof(Result<char>) <= sizeof(void *) * 2);
	static_assert(sizeof(Result<int>) <= sizeof(void *) * 2);
}

// TODO: Remove once clang-format bug is fixed. See above.
// clang-format on

#endif // RAYNI_LIB_FUNCTION_RESULT_H
